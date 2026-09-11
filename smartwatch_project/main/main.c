#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ui.h"
#include "vuvometro.h"
#include "gpio_config.h"
#include "rtc_module.h"
#include "battery_module.h"

static const char *TAG = "esp32_c6_board";

void app_main(void)
{
    gpio_config_init();
    
    ui_draw_text("EDGE Hardware", "System: Booting...");
    vuvometro_start();
    // Sábado 5 de Septiembre de 2026, 12:44:21 PM
    //rtc_set_time(2026, 9, 5, 12, 50, 21);

    char telemetry_buffer[128];
    char time_str[32];

    while(true)
    {
        rtc_get_time_str(time_str, sizeof(time_str));
        // Leemos batería
        int bat_mv = battery_get_voltage_mv();
        int bat_pct = battery_get_percentage();
        
        // Agregamos la batería al texto
        sprintf(telemetry_buffer, "Telemetria: Activa\nTiempo: %s\nBateria: %d mV (%d%%)", 
                time_str, bat_mv, bat_pct);
        ui_draw_text("EDGE Hardware", telemetry_buffer);   
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}