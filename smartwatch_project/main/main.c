#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ui.h"
#include "vuvometro.h"
//#include "bsp_es8311.h"
//#include "esp_codec_dev.h"

#define MOTOR_VIBRATOR_PIN 18
#define BATTERY_ENABLE_PIN 15

static const char *TAG = "esp32_c6_board";

//extern esp_codec_dev_handle_t input_dev;

void init_gpio(void)
{
    gpio_reset_pin(MOTOR_VIBRATOR_PIN);
    gpio_set_direction(MOTOR_VIBRATOR_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BATTERY_ENABLE_PIN);
    gpio_set_direction(BATTERY_ENABLE_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(MOTOR_VIBRATOR_PIN, 0); // Motor apagado
    gpio_set_level(BATTERY_ENABLE_PIN, 1); // Batería habilitada
}

void app_main(void)
{
    init_gpio();
    ui_display_init();
    ui_draw_text("EDGE Hardware", "System: Booting...");
    ESP_LOGI(TAG, "Empezando Audio module...");   
    vuvometro_init();
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