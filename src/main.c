#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>
#include <leds/anim_player.h>
#include <net/ap.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

struct net_if *iface;

frame_result_t anim_idle(uint32_t frame_no, void* _data){
    return  (frame_result_t) {
        .next_frame = K_MSEC(100),
        .led_word = 1 << (frame_no & 7)
    };
}

int main(void)
{
    LOG_INF("Starting AP demo...");

    init_callbacks();

    iface = net_if_get_wifi_sap();
    if (!iface) {
        LOG_ERR("Network interface not found!");
        return -1;
    }
    init_ap(iface);

    // anim_request_t idle_req = {.duration_ms = 2400, .func = anim_idle, .data = NULL};

    while (1) {
        // anim_play(&idle_req);
        k_msleep(5000);
    }


    return 0;
}