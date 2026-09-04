#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ui.h"
#include "bsp_es8311.h"
#include "esp_codec_dev.h"

#define MOTOR_VIBRATOR_PIN 18
#define BATTERY_ENABLE_PIN 15
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 7

static const char *TAG = "esp32_c6_board";

extern esp_codec_dev_handle_t input_dev;

void init_gpio(void)
{
    gpio_reset_pin(MOTOR_VIBRATOR_PIN);
    gpio_set_direction(MOTOR_VIBRATOR_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BATTERY_ENABLE_PIN);
    gpio_set_direction(BATTERY_ENABLE_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(MOTOR_VIBRATOR_PIN, 0); // Motor apagado
    gpio_set_level(BATTERY_ENABLE_PIN, 1); // Batería habilitada
}

static void mic_vu_task(void *arg)
{
    size_t buffer_size = 512 * sizeof(int16_t);
    int16_t *audio_buffer = malloc(buffer_size);

    ESP_LOGI(TAG, "Microphone task started. Make some noise!");

    while (1) {
        // Leemos el audio directamente del driver oficial
        int err = esp_codec_dev_read(input_dev, audio_buffer, buffer_size);
        
        if (err == ESP_CODEC_DEV_OK) {
            int64_t sum = 0;
            int num_samples = buffer_size / sizeof(int16_t);
            for (int i = 0; i < num_samples; i++) {
                sum += abs(audio_buffer[i]);
            }
            int avg_volume = sum / num_samples;

            // Lógica del Vúmetro (ajusta el 20 si es muy sensible o muy sordo)
            int bars = avg_volume / 20; 
            if (bars > 40) bars = 40;
            
            char bar_str[41];
            memset(bar_str, ' ', 40);
            bar_str[40] = '\0';
            for (int i = 0; i < bars; i++) {
                bar_str[i] = '#';
            }

            printf("MIC Vol: %4d | %s\n", avg_volume, bar_str);
            ESP_LOGI(TAG, "MIC Vol: %4d | %s\n", avg_volume, bar_str);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    init_gpio();
    ui_display_init();
    ui_draw_text("EDGE Hardware", "System: Booting...");
    ESP_LOGI(TAG, "Empezando Audio module...");
       
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds

    ESP_LOGI(TAG, "Initializing I2C Master...");
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

    // 4. Inicializar el Codec de Audio (usando tu BSP)
    ESP_LOGI(TAG, "Initializing Audio BSP...");
    bsp_es8311_init(bus_handle);
    bsp_es8311_set_in_volume(40.0); // Ajustar ganancia del micrófono

    // 5. Iniciar la tarea del Vúmetro
    xTaskCreate(mic_vu_task, "mic_vu_task", 4096, NULL, 5, NULL);

    int counter = 0;
    char telemetry_buffer[64];

    while(true)
    {
        sprintf(telemetry_buffer, "Telemetria: Activa\nTiempo: %d seconds", counter);
        ui_draw_text("EDGE Hardware", telemetry_buffer);   
        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}