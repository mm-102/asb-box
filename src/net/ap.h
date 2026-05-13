#pragma once

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

#ifndef WIFI_SSID
#define WIFI_SSID "Sumo_6_AP"
#endif

#ifndef WIFI_PSK
#define WIFI_PSK "sumo1234"
#endif

#ifndef DHCP_BASE_IP
#define DHCP_BASE_IP "192.168.4.2"
#endif

int init_ap(struct net_if *iface);

void init_callbacks();