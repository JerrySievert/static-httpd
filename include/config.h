#pragma once

#include <stdint.h>

// the maximum size of a request to accept
#define MAX_READ_SIZE 65535
#define MAX_PATH_LENGTH 4096
#define MAX_ROOT_LENGTH 512

// configuration parameters that are available to all
extern char *config_root;
extern uint16_t config_root_length;
extern uint16_t config_port;
