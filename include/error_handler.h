#pragma once

#include <stdbool.h>

#include "http_request.h"
#include "http_response.h"

bool serve_404 (Request *, Response *);
bool serve_500 (Request *, Response *);
