#include "buttons.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "logic.h"

LOG_MODULE_REGISTER(buttons, LOG_LEVEL_INF);

#define DEBOUNCE_DELAY_MS 250

static int64_t save_btn_last_time = 0;
static int64_t close_btn_last_time = 0;

#define SAVE_BTN_NODE DT_NODELABEL(grd1)
#define CLOSE_BTN_NODE DT_NODELABEL(grd2)

static const struct gpio_dt_spec save_btn = GPIO_DT_SPEC_GET(SAVE_BTN_NODE, gpios);
static const struct gpio_dt_spec close_btn = GPIO_DT_SPEC_GET(CLOSE_BTN_NODE, gpios);

static struct gpio_callback save_btn_cb_data;
static struct gpio_callback close_btn_cb_data;


static void save_btn_work_handler(struct k_work *work) {
    state_msg_t current_state;
    
    get_curr_state(&current_state);

    if (current_state.code == INPUT_WRITE_CODE) {
        LOG_INF("Save button pressed: Saving state.");
        save_state();
    } else if (current_state.code == INPUT_CHECK_CODE) {
        LOG_INF("Save button pressed: Switching to WRITE_CODE mode.");
        state_msg_t new_state = {
            .code = INPUT_WRITE_CODE,
            .progress = 0
        };
        set_curr_state(&new_state);
    }
}
K_WORK_DEFINE(save_btn_work, save_btn_work_handler);

static void close_btn_work_handler(struct k_work *work) {
    LOG_INF("Close button pressed: Switching to CHECK_CODE mode.");
    state_msg_t new_state = {
        .code = INPUT_CHECK_CODE,
        .progress = 0
    };
    set_curr_state(&new_state);
}
K_WORK_DEFINE(close_btn_work, close_btn_work_handler);

static void save_btn_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    int64_t now = k_uptime_get();
    
    // debounce
    if (now - save_btn_last_time >= DEBOUNCE_DELAY_MS) {
        save_btn_last_time = now;
        k_work_submit(&save_btn_work);
    }
}

static void close_btn_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    int64_t now = k_uptime_get();
    
    // debounce
    if (now - close_btn_last_time >= DEBOUNCE_DELAY_MS) {
        close_btn_last_time = now;
        k_work_submit(&close_btn_work);
    }
}

void buttons_init(void) {
    int ret;

    if (!gpio_is_ready_dt(&save_btn)) {
        LOG_ERR("Error: save button device %s is not ready", save_btn.port->name);
        return;
    }
    if (!gpio_is_ready_dt(&close_btn)) {
        LOG_ERR("Error: close button device %s is not ready", close_btn.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&save_btn, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d", ret, save_btn.port->name, save_btn.pin);
        return;
    }

    ret = gpio_pin_configure_dt(&close_btn, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d", ret, close_btn.port->name, close_btn.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&save_btn, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", ret, save_btn.port->name, save_btn.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&close_btn, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", ret, close_btn.port->name, close_btn.pin);
        return;
    }

    gpio_init_callback(&save_btn_cb_data, save_btn_pressed, BIT(save_btn.pin));
    gpio_add_callback(save_btn.port, &save_btn_cb_data);

    gpio_init_callback(&close_btn_cb_data, close_btn_pressed, BIT(close_btn.pin));
    gpio_add_callback(close_btn.port, &close_btn_cb_data);

    LOG_INF("Hardware buttons initialized successfully.");
}