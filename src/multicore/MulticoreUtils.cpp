#include "multicore/MulticoreUtils.h"

queue_t command_queue;

void send_char_to_core0(char c) {
  queue_add_blocking(&command_queue, &c);
}

char read_char_from_core1() {
  if (queue_is_empty(&command_queue)) {
    return 0x00;
  }
  char c;
  queue_remove_blocking(&command_queue, &c);
  return c;
}