#include "http_response.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static void send_headers (Response *);
static void _write (Response *, uint8_t *);

#define WRITE(response, data) _write(response, (uint8_t *) data);

void _write (Response *response, uint8_t *data) {
  if (response->write != NULL) {
    response->write(data);
  } else {
    size_t size = strlen((char *) data);
    uint8_t res = write(response->fd, (void *) data, size * sizeof(uint8_t));

    if (res) {
      // gcc noop
    }
  }
}

Response *response_create (uint8_t fd) {
  static Response response;

  memset(&response, 0, sizeof(Response));
  response.fd = fd;

  return &response;
}

void response_write (Response *response, uint8_t *data) {
  if (response->headers_sent == 0) {
    send_headers(response);
  }

  WRITE(response, data);
}

void response_sendfile (Response *response, int from) {
  if (response->headers_sent == 0) {
    send_headers(response);
  }

#ifdef __APPLE__
  off_t len = 0;
  int ret = sendfile(from, response->fd, 0, &len, NULL, 0);
#else
  struct stat statbuf = { 0 };
  fstat(from, &statbuf);

  size_t size = statbuf.st_size;

  int ret = sendfile(response->fd, from, NULL, size);
#endif

  if (ret) {
    // noop
  }
}

static const char *response_status (uint16_t code) {
  switch (code) {
    case 200:
    return "HTTP/1.1 200 OK\r\n";
    case 304:
    return "HTTP/1.1 304 NOT MODIFIED\n";
    case 404:
    return "HTTP/1.1 404 NOT FOUND\r\n";
    case 500:
    return "HTTP/1.1 500 ERROR\r\n";
    default:
    return "HTTP/1.1 200 OK\r\n";
  }
}

static void send_headers (Response *response) {
  // if there is no header sent, set it to 200
  if (response->code == 0) {
    response->code = 200;
  }

  response->headers_sent = 1;

  WRITE(response, response_status(response->code));
  WRITE(response, "Content-type: ")

  if (response->content_type[0] != '\0') {
    WRITE(response, response->content_type);
    WRITE(response, "\r\n");
  } else {
    WRITE(response, "text/html\r\n");
  }

  for (uint8_t i = 0; i < response->num_headers; i++) {
    WRITE(response, response->headers[i]);
    WRITE(response, "\r\n");
  }

  WRITE(response, "\r\n");
}

void response_set_header (Response *response, uint8_t *header) {
  if (response->num_headers < MAX_HEADERS - 1) {
    response->headers[response->num_headers] = header;
    response->num_headers++;
  }
}

void response_set_content_type (Response *response, uint8_t *type) {
  uint16_t len = strlen((char *) type);
  if (len < 64) {
    memcpy((void *) response->content_type, (void *) type, len);
  }
}
