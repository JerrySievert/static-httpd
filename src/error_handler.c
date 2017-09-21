#include <unistd.h>
#include <stdint.h>

#include "error_handler.h"

#include "http_request.h"
#include "http_response.h"
#include "path.h"

bool serve_404 (Request *request, Response *response) {
  response->code = 404;
  int fd = open_file_maybe("/404.html", NULL);

  if (fd == -1) {
    response_write(response, (uint8_t *) "404 error");
  } else {
    response_set_content_type(response, (uint8_t *) "text/html");
    response_sendfile(response, fd);
    close(fd);
  }

  return true;
}
