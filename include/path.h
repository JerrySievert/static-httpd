#pragma once

#include <stdbool.h>

#include "http_request.h"
#include "http_response.h"

extern char *search_paths[];

int open_file_maybe (char *, char *);
bool serve_file (Request *, Response *);
