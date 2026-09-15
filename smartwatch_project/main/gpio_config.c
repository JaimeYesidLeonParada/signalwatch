#include "gpio_config.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "vuvometro.h"
#include "rtc_module.h"
#include "ui.h"
#include "button_module.h"
#include "battery_module.h"
#include "touch_module.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ei_wrapper.h"
#include "esp_codec_dev.h"

#define MOTOR_VIBRATOR_PIN 18
#define BATTERY_ENABLE_PIN 15
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 7
#define AUDIO_BUFFER_SIZE 16000

extern esp_codec_dev_handle_t input_dev;

static void audio_inference_task(void *arg) {
    ESP_LOGI("AI_TASK", "Reservando 32KB de memoria para el buffer de audio...");
    
    // Asignamos memoria en el heap para evitar Stack Overflow
    int16_t *audio_buffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
    
    if (audio_buffer == NULL) {
        ESP_LOGE("AI_TASK", "Memoria insuficiente para el buffer de audio!");
        vTaskDelete(NULL);
    }

    // Tamaño total en bytes: 16000 muestras * 2 bytes por muestra = 32000 bytes
    size_t bytes_to_read = AUDIO_BUFFER_SIZE * sizeof(int16_t);

    while(1) {
        ESP_LOGI("AI_TASK", "Escuchando (1 segundo)... ¡Di una palabra clave!");

        // Leemos 1 segundo de audio directamente del driver oficial ES8311
        int err = esp_codec_dev_read(input_dev, (void *)audio_buffer, bytes_to_read);
        
        if (err == ESP_CODEC_DEV_OK) {
            // ¡Magia! Le pasamos el buffer a la red neuronal
            ei_wrapper_classify(audio_buffer, AUDIO_BUFFER_SIZE);
        } else {
            ESP_LOGE("AI_TASK", "Error leyendo el microfono: %d", err);
        }

        // Pequeña pausa para no saturar la CPU
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

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

    vuvometro_init(bus_handle);
    rtc_init(bus_handle);
    touch_init(bus_handle);

    button_init();
    battery_init();

    xTaskCreate(audio_inference_task, "ai_task", 8192, NULL, 5, NULL);
}

void gpio_config_init(void)
{
    gpio_reset_pin(MOTOR_VIBRATOR_PIN);
    gpio_set_direction(MOTOR_VIBRATOR_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BATTERY_ENABLE_PIN);
    gpio_set_direction(BATTERY_ENABLE_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(MOTOR_VIBRATOR_PIN, 0); // Motor apagado
    gpio_set_level(BATTERY_ENABLE_PIN, 1); // Batería habilitada

    config_init_modules();
}