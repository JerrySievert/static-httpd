#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "http_request.h"
#include "config.h"

Request *parse_request (uint8_t *request) {
  static Request req = {0};
  uint16_t position = 0;

  // start iterating through the request, break out the parts.
  // this is clunky, but fast
  if (request[0] == 'G' && request[1] == 'E' && request[2] == 'T' && request[3] == ' ') {
    position = 4;
    req.method = HTTP_GET;
  } else if (request[0] == 'P') {
    if (request[1] == 'O' && request[2] == 'S' && request[3] == 'T' && request[4] == ' ') {
      position = 5;
      req.method = HTTP_POST;
    } else if (request[1] == 'U' && request[2] == 'T' && request[3] == ' ') {
      position = 4;
      req.method = HTTP_PUT;
    } else {
      req.method = HTTP_ERROR;
      return &req;
    }
  } else if (request[0] == 'D' && request[1] == 'E' && request[2] == 'L' && request[4] == 'E' &&
             request[5] == 'T' && request[6] == 'E' && request[7] == ' ') {
    req.method = HTTP_DELETE;
    return &req;
  } else {
    req.method = HTTP_ERROR;
    return &req;
  }

  // set the uri to the current position, now that we have the method
  uint8_t *uri = &request[position];


  while (request[position] != '\0' && request[position] != ' ' && request[position] != '\n' && request[position] != '\r') {
    position++;
  }

  // if we're already at the end, we can consider this an invalid request
  if (request[position] == '\0') {
    req.method = HTTP_ERROR;

    return &req;
  }

  // set the uri by terminating the string where we left off
  request[position] = '\0';

  // check that the uri is no longer than the MAX_PATH_LENGTH
  if (strlen((char *) uri) >= MAX_PATH_LENGTH) {
    uri[MAX_PATH_LENGTH - 1] = '\0';
  }

  req.path = uri;

  // and set up the params
  parse_params(&req, uri);

  position++;

  while (request[position] != '\n' && request[position] != '\r' && request[position] != '\0') {
    position++;
  }

  if (request[position] == '\0') {
    return &req;
  }

  request[position] = '\0';

  position++;

  // move past the LF
  if (request[position + 1] == '\r' || request[position + 1] == '\n') {
    position++;
  }

  position++;

  if (request[position] == '\0') {
    return &req;
  }

  parse_headers(&req, &request[position]);

  return &req;
}

void parse_params (Request *req, uint8_t *uri) {
  static uint8_t *params[MAX_PARAMS + 1];
  uint8_t current = 0;

  uint16_t position = 0;

  // figure out where the params start
  while (uri[position] != '\0' && uri[position] != '?') {
    position++;
  }

  if (uri[position] == '?') {
    uri[position] = '\0';
    position++;

    if (uri[position] == '\0') {
      // uri ends with ? and nothing further
      return;
    }
  } else {
    // at the end, no params return NULL
    return;
  }

  // assign the first parameter and move forward
  params[current] = &uri[position];
  current++;

  while (uri[position] != '\0') {
    if (uri[position] == '&') {
      uri[position] = '\0';

      params[current] = &uri[position + 1];
      current++;
      if (uri[position + 1] == '\0') {
        break;
      }
    }

    position++;
  }

  params[current] = NULL;

  req->params = params;
}

void parse_headers (Request *req, uint8_t *request) {
  static uint8_t *headers[MAX_HEADERS + 1];
  uint16_t current = 0;

  if (request[0] == '\0') {
    return;
  }

  uint16_t position = 0;

  headers[current] = request;
  current++;

  while (request[position] != '\0') {
    if (request[position] == '\n' || request[position] == '\r') {
      request[position] = '\0';
      position++;

      if (request[position] == '\r' || request[position] == '\n') {
        if (request[position + 1] == '\r' || request[position + 1] == '\n') {
          break;
        } else {
          position++;
        }
      }

      if (request[position] != '\0') {
        headers[current] = &request[position];
        current++;
      }
    }

    position++;
  }

  headers[current] = NULL;

  req->headers = headers;

  if (request[position] != '\0') {
    position++;
  }

  if (request[position] != '\0') {
    position++;
  }

  req->body = &request[++position];
}
