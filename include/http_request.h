#pragma once

#include <stdint.h>

#ifndef MAX_PARAMS
#define MAX_PARAMS 25
#endif

#ifndef MAX_HEADERS
#define MAX_HEADERS 25
#endif

#ifndef MAX_HEADER_LENGTH
#define MAX_HEADER_LENGTH 512
#endif

#define HTTP_ERROR 0
#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_PUT 3
#define HTTP_DELETE 4

typedef struct Request {
  uint8_t method;
  uint8_t **params;
  uint8_t *path;
  uint8_t **headers;
  uint8_t *body;
} Request;

Request *parse_request (uint8_t *);
void parse_params (Request *, uint8_t *);
void parse_headers (Request *, uint8_t *);
