#include "logic.h"

#include <assert.h>
#include <leds/anim_lib.h>
#include <leds/anim_player.h>
#include <math.h>
#include <nvs/nvs_manager.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(logic, LOG_LEVEL_INF);

void log_state(const char prefix[], const state_msg_t *state) {
  LOG_INF("%s State: [code: %d, progress: %d]", prefix, state->code,
          state->progress);
}

void log_entry(const char prefix[], const entry_msg_t *entry) {
  LOG_INF("%s Entry: [name: %s, buttons: %d, R:%.2f P:%.2f Y:%.2f]", prefix,
          entry->name, entry->buttons, (double)entry->euler_angles[0],
          (double)entry->euler_angles[1], (double)entry->euler_angles[2]);
}

#define EULER_TOLERANCE 5.0f

int entry_eq(const entry_msg_t *a, const entry_msg_t *b) {
  log_entry("CMP A", a);
  log_entry("CMP B", b);
  if (strncmp(a->name, b->name, sizeof(a->name)) != 0) {
    return 0; /* Names are different */
  }

  if (a->buttons != b->buttons) {
    return 0; /* Buttons are different */
  }

  for (int i = 0; i < 3; i++) {
    if (fabsf(a->euler_angles[i] - b->euler_angles[i]) > EULER_TOLERANCE) {
      return 0; /* Angle difference exceeded tolerance */
    }
  }
  return 1;
}

static state_msg_t curr_state;

static entry_msg_t curr_code[MAX_ENTRIES];
static int curr_code_len;

static input_mode_t curr_mode;

void logic_init() {
  curr_state.progress = 0;

  curr_code_len = nvs_manager_load_entries(curr_code);
  curr_code_len = curr_code_len > 0 ? curr_code_len : 0;

  if (curr_code_len > 0) {
    curr_mode = INPUT_CHECK_CODE;
    curr_state.code = INPUT_CHECK_CODE;
    LOG_INF("Init mode CHECK_CODE");
    anim_request_t req = {-1, anim_progress_check, 0};
    anim_play(&req);
  } else {
    curr_mode = INPUT_WRITE_CODE;
    curr_state.code = INPUT_WRITE_CODE;
    LOG_INF("Init mode WRITE_CODE");
    anim_request_t req = {-1, anim_progress_write, 0};
    anim_play(&req);
  }
}

void get_curr_state(state_msg_t *state) { *state = curr_state; }

void reset_state(state_msg_t *state) {
  anim_request_t req;
  req.duration_ms = -1;

  switch (curr_mode) {
  case INPUT_WRITE_CODE:
    req.func = anim_progress_write;
    req.data = 0;
    break;
  case INPUT_CHECK_CODE:
  default:
    req.func = anim_reset;
    req.data = (void *)(uint32_t)curr_state.progress;
    break;
  }

  anim_play(&req);
  curr_state.progress = 0;
  *state = curr_state;
  LOG_INF("State Reset!");
}

void set_curr_state(const state_msg_t *state) {
  if (state->code >= INPUT_MODE_SIZE) {
    log_state("Tried to manually set invalid state:", state);
    return;
  }
  curr_state = *state;
  curr_mode = (input_mode_t)curr_state.code;
  log_state("State set manually:", &curr_state);
  anim_request_t req;
  req.duration_ms = -1;
  req.data = (void *)(uint32_t)curr_state.progress;

  switch (curr_mode) {
  case INPUT_WRITE_CODE:
    curr_code_len = curr_state.progress;
    req.func = anim_progress_write;
    break;
  case INPUT_CHECK_CODE:
  default:
    req.func = anim_progress_check;
    break;
  }
  anim_play(&req);
}

void recv_entry(const entry_msg_t *entry, state_msg_t *state) {
  log_entry("Recv:", entry);

  switch (curr_mode) {
  case INPUT_WRITE_CODE: {
    if (curr_code_len < MAX_ENTRIES) {
      LOG_INF("Entry added to code at pos: %d", curr_code_len);
      curr_code[curr_code_len++] = *entry;
      curr_state.progress = curr_code_len;
      anim_request_t req = {-1, anim_progress_write,
                            (void *)(uint32_t)curr_state.progress};
      anim_play(&req);
    } else {
      LOG_WRN("Tried to add too many entries to the code!");
    }

  } break;
  case INPUT_CHECK_CODE:
  default: {
    if (curr_state.progress >= curr_code_len) {
      LOG_WRN("Invalid state of logic module!");
    } else if (entry_eq(&curr_code[curr_state.progress], entry)) {
      curr_state.progress++;
      LOG_INF("Correct! current progress: %d", curr_state.progress);
      if (curr_state.progress == curr_code_len) {
        anim_request_t req = {3000, anim_victory, 0};
        anim_play(&req);
        // TODO run the async victory code here
        curr_state.progress = 0;
        LOG_INF("Safe open!");
      } else {
        anim_request_t req = {-1, anim_progress_check,
                              (void *)(uint32_t)curr_state.progress};
        anim_play(&req);
      }
    } else {
      LOG_INF("Mistake! progress will reset");
      reset_state(state);
    }
  } break;
  }

  *state = curr_state;
}