#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ui.h"
#include "vuvometro.h"
#include "gpio_config.h"


static const char *TAG = "esp32_c6_board";

void app_main(void)
{
    gpio_config_init();
    ui_display_init();
    vuvometro_init();

    ui_draw_text("EDGE Hardware", "System: Booting...");
    vuvometro_start();

    int counter = 0;
    char telemetry_buffer[64];

    while(true)
    {
        sprintf(telemetry_buffer, "Telemetria: Activa\nTiempo: %d segundos", counter);
        ui_draw_text("EDGE Hardware", telemetry_buffer);   
        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}