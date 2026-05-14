#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>

#include <net/ap.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

struct net_if *iface;

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
    
    while (1) {
        k_sleep(K_SECONDS(5));
        LOG_INF("Alive...");
    }

    return 0;
}