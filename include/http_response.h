#pragma once

#include <stdint.h>

#include "http_request.h"

typedef struct Response {
  uint16_t code;
  uint8_t *headers[MAX_HEADERS];
  uint8_t num_headers;
  uint8_t content_type[64];
  uint8_t headers_sent;
  uint8_t fd;
  void (*write)(uint8_t *);
} Response;

Response *response_create (uint8_t);

void response_write (Response *, uint8_t *);
void response_set_header (Response *, uint8_t *);
void response_set_content_type (Response *, uint8_t *);
void response_sendfile (Response *, int);
