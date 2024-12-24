#include "MulticoreUtils.h"

queue_t command_queue;

void send_string_to_core0(const char *large_string) {
  size_t len = strlen(large_string) + 1;
  if (len > MAX_COMMAND_SIZE) {
    // Handle error: string too long
    return;
  }
  if (queue_is_full(&command_queue)) {
    // Handle error: queue full
    return;
  }
  queue_add_blocking(&command_queue, large_string);
}

char received_string[MAX_COMMAND_SIZE];
char *receive_string_from_core1() {
  if (queue_is_empty(&command_queue)) {
    // Handle error: queue empty, return last command
    return received_string;
  }
  queue_remove_blocking(&command_queue, received_string);
  return received_string;
}