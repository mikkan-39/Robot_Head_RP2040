#ifndef _MULTICORE_UTILS_
#define _MULTICORE_UTILS_

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <cstdio>
#include <cstring>

#define MAX_COMMAND_SIZE                                   \
  128 // Max size that fits in FIFO incl. null terminator

extern queue_t command_queue;
void send_string_to_core0(const char *large_string);
char *receive_string_from_core1();

#endif
