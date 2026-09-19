#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ui.h"
#include "vuvometro.h"
#include "gpio_config.h"
#include "rtc_module.h"
#include "battery_module.h"
#include "ei_wrapper.h"
#include "bsp_es8311.h"

static const char *TAG = "esp32_c6_board";

void app_main(void)
{
    gpio_config_init();
    //vuvometro_start();
    // Sábado 5 de Septiembre de 2026, 12:44:21 PM
    //rtc_set_time(2026, 9, 5, 12, 50, 21);

    char time_str[32];

    ei_wrapper_init();

    while(true)
    {
        rtc_get_time_str(time_str, sizeof(time_str));
        int bat_pct = battery_get_percentage();
        
        ui_post_clock_msg(time_str);
        ui_post_battery_msg(bat_pct);  
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}