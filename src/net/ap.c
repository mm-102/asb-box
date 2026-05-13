#include "ap.h"
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(ap, LOG_LEVEL_INF);

static struct net_mgmt_event_callback wifi_mgmt_cb;

int init_ap(struct net_if *iface) {
  // clang-format off
  struct wifi_connect_req_params ap_params = {
    .ssid = WIFI_SSID,
    .ssid_length = strlen(WIFI_SSID),
    .psk = WIFI_PSK,
    .psk_length = strlen(WIFI_PSK),
    .channel = 6,
    .security = WIFI_SECURITY_TYPE_PSK
  };
  // clang-format on

  LOG_INF("Starting Wi-Fi Access Point...");

  int ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, iface, &ap_params,
                     sizeof(ap_params));
  if (ret) {
    LOG_ERR("Failed to start AP (err: %d)", ret);
    return ret;
  }

  LOG_INF("AP Started successfully! SSID: %s", WIFI_SSID);

  struct net_in_addr base_addr;
  ret = net_addr_pton(AF_INET, DHCP_BASE_IP, &base_addr);
  if (ret) {
    LOG_ERR("Invalid IPv4 string format");
    return ret;
  }

  ret = net_dhcpv4_server_start(iface, &base_addr);
  if (ret) {
    LOG_ERR("Failed to start DHCP server (err: %d)", ret);
    return ret;
  }

  LOG_INF("DHCP Server started! IP pool begins at " DHCP_BASE_IP);
  return 0;
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event, struct net_if *iface) {
  const struct wifi_ap_sta_info *sta_info =
      (const struct wifi_ap_sta_info *)cb->info;

  if (mgmt_event == NET_EVENT_WIFI_AP_STA_CONNECTED) {
    LOG_INF("New client connected! MAC: %02x:%02x:%02x:%02x:%02x:%02x",
            sta_info->mac[0], sta_info->mac[1], sta_info->mac[2],
            sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
  }
  if (mgmt_event == NET_EVENT_WIFI_AP_STA_DISCONNECTED) {
    LOG_INF("New client connected! MAC: %02x:%02x:%02x:%02x:%02x:%02x",
            sta_info->mac[0], sta_info->mac[1], sta_info->mac[2],
            sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
  }
}

void init_callbacks() {

  net_mgmt_init_event_callback(&wifi_mgmt_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_AP_STA_CONNECTED |
                                   NET_EVENT_WIFI_AP_STA_DISCONNECTED);
    
  net_mgmt_add_event_callback(&wifi_mgmt_cb);
}