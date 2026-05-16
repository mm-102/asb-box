#pragma once

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

#define REG_74HC595_SPEC(node_id) SPI_DT_SPEC_GET(node_id, SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB)

int reg_74hc595_init(struct spi_dt_spec *reg);
int reg_74hc595_write(const struct spi_dt_spec *reg, uint8_t word);