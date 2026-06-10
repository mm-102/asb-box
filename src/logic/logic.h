#pragma once
#include <stdint.h>

#define MAX_ENTRIES 8

typedef struct __attribute__((packed)) {
  char name[8];
  uint32_t buttons;
  float euler_angles[3];
} entry_msg_t;

typedef struct __attribute__((packed)) {
  uint8_t code;
  uint8_t progress;
} state_msg_t;

typedef enum {
  INPUT_WRITE_CODE,
  INPUT_CHECK_CODE,
  INPUT_MODE_SIZE
} input_mode_t;

void logic_init();
void get_curr_state(state_msg_t *state);
void set_curr_state(const state_msg_t *state);
void reset_state(state_msg_t *state);
void save_state(void);
void recv_entry(const entry_msg_t *entry, state_msg_t *state);