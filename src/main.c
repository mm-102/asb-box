#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <net/ap.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

struct net_if *iface;

int main(void)
{
    LOG_INF("Starting AP demo...");

    iface = net_if_get_default();
    if (!iface) {
        LOG_ERR("Network interface not found!");
        return -1;
    }
    init_callbacks();
    init_ap(iface);
    
    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}