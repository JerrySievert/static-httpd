#pragma once

#include "mime_types.h"

extern char *mime_extensions[];
extern char *mime_types[];

char *mime_type_find (char *);
char *mime_type_from_filename (char *);
