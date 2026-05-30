#include <stdint.h>

typedef struct __attribute__((packed)) {
  char name[8];
  uint32_t buttons;
  float euler_angles[3];
} entry_msg_t;

typedef struct __attribute__((packed)) {
  uint8_t code;
  uint8_t progress;
} state_msg_t;

void get_curr_state(state_msg_t *state);
void reset_state(state_msg_t *state);
void recv_entry(const entry_msg_t *entry, state_msg_t *state);