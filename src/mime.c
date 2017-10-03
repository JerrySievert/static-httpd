#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mime.h"
#include "mime_types.h"
#include "deps/strmap/strmap.h"

static bool is_init = false;
static StrMap *sm = NULL;

static void init_mime ( ) {
  if (is_init) {
    return;
  }

  sm = sm_new(26);

  if (sm != NULL) {
    for (uint16_t i = 0; i < MIME_LENGTH; i++) {
      sm_put(sm, mime_extensions[i], mime_types[i]);
    }

    is_init = true;
  }
}

char *mime_type_find (char *extension) {
  if (!is_init) {
    init_mime();
  }

  static char ret[512];
  int res = sm_get(sm, extension, ret, 512);

  if (res) {
    return ret;
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
