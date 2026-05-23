#include "anim_player.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <drivers/reg_74hc595.h>
#include <stdint.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(anim_player, LOG_LEVEL_INF);

K_MSGQ_DEFINE(anim_msgq, sizeof(anim_request_t), 10, alignof(anim_request_t));

static struct spi_dt_spec shift_reg_spi = REG_74HC595_SPEC(DT_ALIAS(shift_reg));

void anim_play(const anim_request_t *request){
    if(k_msgq_put(&anim_msgq, request, K_NO_WAIT)){
        k_msgq_purge(&anim_msgq);
        k_msgq_put(&anim_msgq, &request, K_NO_WAIT);
    }
}

frame_result_t anim_off(uint32_t _frame_no, void* _data){
    return (frame_result_t) {K_FOREVER,0};
}
static anim_request_t anim_off_req = {.duration_ms = -1, .func = anim_off, .data = NULL};

void anim_thread_main(void *p1, void *p2, void *p3){
    if (reg_74hc595_init(&shift_reg_spi) < 0){
        LOG_ERR("Could not init LED driver");
    } else {
        LOG_INF("Player started...");
        anim_request_t curr_anim = anim_off_req;
        anim_request_t new_req;
        frame_result_t res;
        uint32_t frame_no;
        int64_t anim_start = k_uptime_get();
        k_timeout_t frame_delay = K_NO_WAIT;

        while (1) {
            int ret = k_msgq_get(&anim_msgq, &new_req, frame_delay);

            // new anim req
            if (ret == 0){
                curr_anim = new_req;
                frame_no = 0;
                anim_start = k_uptime_get();
            }

            // time to stop
            if(curr_anim.duration_ms != -1){
                int64_t elapsed = k_uptime_get() - anim_start;
                if (elapsed >= curr_anim.duration_ms) {
                    curr_anim = anim_off_req;
                    frame_no = 0;
                }
            }

            res = curr_anim.func(frame_no++, curr_anim.data);
            frame_delay = res.next_frame;
            reg_74hc595_write(&shift_reg_spi, res.led_word);
            LOG_DBG("writing frame");
        }
    }
}

K_THREAD_DEFINE(anim_tid, 1024, anim_thread_main, NULL, NULL, NULL, 5, 0, 0);