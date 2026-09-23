#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "ui.h"
#include "gpio_config.h"
#include "rtc_module.h"


void app_main(void)
{
    gpio_config_init();
    //vuvometro_start();
    // 23 de Septiembre de 2026, 2:30:21 PM
    //rtc_set_time(2026, 9, 23, 14, 30, 21);

    char time_str[32];

    while(true)
    {
        rtc_get_time_str(time_str, sizeof(time_str));        
        ui_post_clock_msg(time_str);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}