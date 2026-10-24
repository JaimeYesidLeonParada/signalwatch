#include "gpio_config.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "ei_wrapper.h"
#include "bsp_es8311.h"
#include "rtc_module.h"
#include "ui.h"
#include "button_module.h"
#include "battery_module.h"
#include "touch_module.h"
#include "ai_module.h"
#include "alert_dispatcher.h"
#include "vibrator_module.h"

#define BATTERY_ENABLE_PIN 15
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 7

void config_init_modules(void)
{
    ui_display_init();
    
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_PIN,
        .sda_io_num = I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    bsp_es8311_init(bus_handle);
    bsp_es8311_set_in_volume(40.0);
    rtc_init(bus_handle);
    touch_init(bus_handle);

    button_init();
    battery_init(ui_post_battery_msg);
    vibrator_init();

    ei_wrapper_init();
    alert_dispatcher_init();
    const ai_listener_t ai_listener = {
        .on_audio_level    = alert_dispatcher_on_audio_level,
        .on_classification = alert_dispatcher_on_classification,
    };
    ai_module_start(bsp_es8311_get_input_dev(), &ai_listener);
    rtc_task_start(ui_post_clock_msg);
    vibrator_task_start();
}

void gpio_config_init(void)
{
    gpio_reset_pin(BATTERY_ENABLE_PIN);
    gpio_set_direction(BATTERY_ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BATTERY_ENABLE_PIN, 1); // Batería habilitada

    config_init_modules();
}