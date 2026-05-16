#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>
#include <peripheral/reg_74hc595.h>
#include <net/ap.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

struct net_if *iface;
static struct spi_dt_spec shift_reg_spi = REG_74HC595_SPEC(DT_ALIAS(shift_reg));

int main(void)
{
    LOG_INF("Starting AP demo...");

    if (reg_74hc595_init(&shift_reg_spi) < 0){
        return 0;
    }

    init_callbacks();

    iface = net_if_get_wifi_sap();
    if (!iface) {
        LOG_ERR("Network interface not found!");
        return -1;
    }
    init_ap(iface);
    
    uint8_t data_out = 0x01; 
    bool shift_left = true;
    
    while (1) {
        reg_74hc595_write(&shift_reg_spi, data_out);

        if (shift_left) {
            data_out <<= 1;
            if (data_out == 0x80) { 
                shift_left = false;
            }
        } else {
            data_out >>= 1;
            if (data_out == 0x01) { 
                shift_left = true;
            }
        }

        k_msleep(100);
    }

    return 0;
}