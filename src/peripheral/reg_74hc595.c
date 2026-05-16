#include "reg_74hc595.h"
#include <sys/errno.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(reg_74hc595, LOG_LEVEL_INF);

int reg_74hc595_init(struct spi_dt_spec *reg) {
  if (!spi_is_ready_dt(reg)) {
    LOG_ERR("%s : SPI device not ready", reg->bus->name ? reg->bus->name : "unknown");
    return -ENODEV;
  }
  LOG_DBG("%s : Init OK", reg->bus->name ? reg->bus->name : "unknown");
  return 0;
}

int reg_74hc595_write(const struct spi_dt_spec *reg, uint8_t word) {
  struct spi_buf tx_buf = {.buf = &word, .len = sizeof(word)};
  struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
  int ret = spi_write_dt(reg, &tx_bufs);
  if(ret < 0){
    LOG_WRN_RATELIMIT("%s : SPI Write failed, code: %d", reg->bus->name ? reg->bus->name : "unknown", ret);
  }
  return ret;
}