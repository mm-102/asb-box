#pragma once

#include "anim_player.h"
#include "zephyr/kernel.h"
#include <assert.h>
#include <stdint.h>

frame_result_t anim_progress_check(uint32_t frame_no, void *data) {
  static_assert(sizeof(uint32_t) == sizeof(void *), "non 4 bytes ptr");
  uint8_t val = (uint32_t)data;
  uint8_t word = (1 << val) - 1;

  if (frame_no & 1)
    word |= (1 << val);

  return (frame_result_t){.next_frame = K_MSEC(500), .led_word = word};
}

frame_result_t anim_reset(uint32_t frame_no, void *data) {
  static_assert(sizeof(uint32_t) == sizeof(void *), "non 4 bytes ptr");

  if (frame_no > (uint32_t)data) {
    return anim_progress_check(frame_no, 0);
  }
  uint8_t val = (uint32_t)data;
  return (frame_result_t){.next_frame = K_MSEC(100),
                          .led_word = (1 << (val - frame_no)) - 1};
}

frame_result_t anim_progress_write(uint32_t frame_no, void *data) {
  static_assert(sizeof(uint32_t) == sizeof(void *), "non 4 bytes ptr");
  uint8_t val = (uint32_t)data;
  uint8_t rest = 8 - val;
  uint8_t word = (1 << val) - 1;

  uint8_t shift = val + (frame_no % rest);
  word |= (1 << shift);
  return (frame_result_t){.next_frame = K_MSEC(150), .led_word = word};
}

frame_result_t anim_victory(uint32_t frame_no, void *data) {
    return (frame_result_t) {
        .next_frame = K_MSEC(500),
        .led_word = (frame_no & 1 ? 0xff : 0x0)
    };
}