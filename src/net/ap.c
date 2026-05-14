#include "ap.h"
#include "zephyr/net/wifi.h"
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(ap, LOG_LEVEL_INF);

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

static struct net_mgmt_event_callback wifi_mgmt_cb;

void enable_dhcp(struct net_if *ap_iface) {
  static struct net_in_addr addr;
  static struct net_in_addr netmaskAddr;

  if (net_addr_pton(NET_AF_INET, CONFIG_APP_AP_IP_ADDRESS, &addr)) {
    LOG_ERR("Invalid address: %s", CONFIG_APP_AP_IP_ADDRESS);
    return;
  }

  if (net_addr_pton(NET_AF_INET, CONFIG_APP_AP_NETMASK, &netmaskAddr)) {
    LOG_ERR("Invalid netmask: %s", CONFIG_APP_AP_NETMASK);
    return;
  }

  net_if_ipv4_set_gw(ap_iface, &addr);

  if (net_if_ipv4_addr_add(ap_iface, &addr, NET_ADDR_MANUAL, 0) == NULL) {
    LOG_ERR("unable to set IP address for AP interface");
  }

  if (!net_if_ipv4_set_netmask_by_addr(ap_iface, &addr, &netmaskAddr)) {
    LOG_ERR("Unable to set netmask for AP interface: %s",
            CONFIG_APP_AP_NETMASK);
  }

  addr.s4_addr[3] += 10; /* Starting IPv4 address for DHCPv4 address pool. */

  if (net_dhcpv4_server_start(ap_iface, &addr) != 0) {
    LOG_ERR("DHCP server is not started for desired IP");
    return;
  }

  LOG_INF("DHCPv4 server started...\n");
}

int init_ap(struct net_if *iface) {
  // clang-format off
  struct wifi_connect_req_params ap_params = {
    .ssid = CONFIG_APP_WIFI_SSID,
    .ssid_length = strlen(CONFIG_APP_WIFI_SSID),
    .psk = CONFIG_APP_WIFI_PSK,
    .psk_length = strlen(CONFIG_APP_WIFI_PSK),
    .band = WIFI_FREQ_BAND_2_4_GHZ,
    .channel = WIFI_CHANNEL_ANY,
    .security = WIFI_SECURITY_TYPE_PSK
  };
  // clang-format on

  enable_dhcp(iface);

  LOG_INF("Starting Wi-Fi Access Point...");

  int ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, iface, &ap_params,
                     sizeof(ap_params));
  if (ret) {
    LOG_ERR("Failed to start AP (err: %d)", ret);
    return ret;
  }
  return 0;
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event, struct net_if *iface) {
  switch (mgmt_event) {
  case NET_EVENT_WIFI_AP_STA_CONNECTED: {
    struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

    LOG_INF("station: " MACSTR " joined ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4],
            sta_info->mac[5]);
    break;
  }
  case NET_EVENT_WIFI_AP_STA_DISCONNECTED: {
    struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

    LOG_INF("station: " MACSTR " leave ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4],
            sta_info->mac[5]);
    break;
  }
  default:
    break;
  }
}

void init_callbacks() {

  net_mgmt_init_event_callback(&wifi_mgmt_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_AP_STA_CONNECTED |
                                   NET_EVENT_WIFI_AP_STA_DISCONNECTED);

  net_mgmt_add_event_callback(&wifi_mgmt_cb);
}