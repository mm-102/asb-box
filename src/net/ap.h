#pragma once

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

int init_ap(struct net_if *iface);

void init_callbacks();