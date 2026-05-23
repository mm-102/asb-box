#pragma once
#include <stdint.h>
#include <zephyr/sys/clock.h>
#include <stdalign.h>
#include <zephyr/kernel.h>

typedef struct {
    k_timeout_t next_frame;
    uint8_t led_word;
} frame_result_t;

typedef frame_result_t (*anim_frame_func_t)(uint32_t frame_no, void* data);

typedef struct {
    int duration_ms;
    anim_frame_func_t func;
    void* data;
} anim_request_t;

frame_result_t anim_off(uint32_t,void*);

void anim_play(const anim_request_t *request);