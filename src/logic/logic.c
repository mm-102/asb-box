#include "logic.h"

#include <assert.h>
#include <leds/anim_player.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(logic, LOG_LEVEL_INF);

static frame_result_t anim_progress(uint32_t frame_no, void *data) {
  static_assert(sizeof(uint32_t) == sizeof(void *), "non 4 bytes ptr");
  uint8_t val = (uint32_t)data;
  uint8_t word = (1 << val) - 1;

  if (frame_no & 1)
    word |= (1 << val);

  return (frame_result_t){.next_frame = K_MSEC(500), .led_word = word};
}

static frame_result_t anim_reset(uint32_t frame_no, void *data) {
  static_assert(sizeof(uint32_t) == sizeof(void *), "non 4 bytes ptr");

  if (frame_no > (uint32_t)data) {
    return anim_progress(frame_no, 0);
  }
  uint8_t val = (uint32_t)data;
  return (frame_result_t){.next_frame = K_MSEC(100),
                          .led_word = (1 << (val - frame_no)) - 1};
}

static state_msg_t global_state = {.code = 0, .progress = 0};
void get_curr_state(state_msg_t *state) { *state = global_state; }

void reset_state(state_msg_t *state) {
  anim_request_t req = {-1, anim_reset,
                        (void *)(uint32_t)global_state.progress};
  anim_play(&req);
  global_state.code = 0;
  global_state.progress = 0;
  *state = global_state;
  LOG_INF("State Reset!");
}
void recv_entry(const entry_msg_t *entry, state_msg_t *state) {
  LOG_INF("Entry from %s | Buttons: %d | R:%.2f P:%.2f Y:%.2f", entry->name,
          entry->buttons, (double)entry->euler_angles[0],
          (double)entry->euler_angles[1], (double)entry->euler_angles[2]);

  global_state.progress++;
  global_state.code = global_state.progress & 1;
  *state = global_state;
  anim_request_t req = {.duration_ms = -1,
                        .func = anim_progress,
                        .data = (void *)(uint32_t)global_state.progress};
  anim_play(&req);
}