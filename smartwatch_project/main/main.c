#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#include "ui.h"
#include "vuvometro.h"
#include "gpio_config.h"
#include "rtc_module.h"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 7

static const char *TAG = "esp32_c6_board";

void app_main(void)
{
    gpio_config_init();
    ui_display_init();
    
    ESP_LOGI(TAG, "Initializing Shared I2C Master...");
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

    vuvometro_init(bus_handle);
    rtc_init(bus_handle);

    ui_draw_text("EDGE Hardware", "System: Booting...");
    vuvometro_start();

    char telemetry_buffer[64];
    char time_str[32];

    while(true)
    {
        // Leer la hora del RTC (creado en el paso anterior)
        rtc_get_time_str(time_str, sizeof(time_str));
        
        sprintf(telemetry_buffer, "Telemetria: Activa\nTiempo: %s", time_str);
        ui_draw_text("EDGE Hardware", telemetry_buffer);   
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}