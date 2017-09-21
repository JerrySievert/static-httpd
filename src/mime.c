#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "mime.h"
#include "mime_types.h"

char *mime_type_find (char *extension) {
  for (uint16_t i = 0; i < MIME_LENGTH; i++) {
    if (strcmp(mime_extensions[i], extension) == 0) {
      return mime_types[i];
    }
  }

  return "application/octet-stream";
}

char *mime_type_from_filename (char *filename) {
  char *ext = strrchr(filename, '.');
  if (!ext) {
    return "application/octet-stream";
  }  else {
    ext++;
    return mime_type_find(ext);
  }
}
