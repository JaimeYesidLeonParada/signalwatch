#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "ui.h"
#include "gpio_config.h"
#include "rtc_module.h"
#include "battery_module.h"


void app_main(void)
{
    gpio_config_init();
    //vuvometro_start();
    // 22 de Septiembre de 2026, 4:54:21 PM
    rtc_set_time(2026, 9, 22, 16, 54, 21);

    char time_str[32];

    while(true)
    {
        rtc_get_time_str(time_str, sizeof(time_str));
        int bat_pct = battery_get_percentage();
        
        ui_post_clock_msg(time_str);
        ui_post_battery_msg(bat_pct);  
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}