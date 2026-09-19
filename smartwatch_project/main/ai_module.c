#include "ai_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "ei_wrapper.h"
#include <stdlib.h>
#include "esp_timer.h"
#include "ui.h"

#define AUDIO_BUFFER_SIZE   16000   // 1 s @ 16 kHz (lo que exige el modelo)
#define CHUNK_SAMPLES       4000    // 0.25 s
#define CHUNKS_PER_WINDOW   (AUDIO_BUFFER_SIZE / CHUNK_SAMPLES)
#define AI_TASK_PRIORITY        5
#define AI_TASK_PRIORITY_BUSY   0  

static const char *TAG = "ai_module";

// Import from the audio BSP
extern esp_codec_dev_handle_t input_dev;


static void audio_inference_task(void *arg) {
    int16_t *audio_buffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));

    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "Memoria insuficiente para el buffer de audio!");
        vTaskDelete(NULL);
        return;
    }

    const size_t chunk_bytes = CHUNK_SAMPLES * sizeof(int16_t);
    int avg_intensity[CHUNKS_PER_WINDOW];

    while (1) {
        ESP_LOGI(TAG, "Escuchando (%d trozos de 0.25 s)...", CHUNKS_PER_WINDOW);

        bool window_ok = true;
        int64_t t_start = esp_timer_get_time();

        for (int c = 0; c < CHUNKS_PER_WINDOW; c++) {
            int16_t *chunk = audio_buffer + (c * CHUNK_SAMPLES);

            int err = esp_codec_dev_read(input_dev, (void *)chunk, chunk_bytes);

            if (err != ESP_CODEC_DEV_OK) {
                ESP_LOGE(TAG, "Error leyendo microfono en el trozo %d: %d", c, err);
                window_ok = false;
                break;
            }

            int64_t sum = 0;
            for (int i = 0; i < CHUNK_SAMPLES; i++) {
                sum += abs(chunk[i]);
            }
            avg_intensity[c] = (int)(sum / CHUNK_SAMPLES);

            int level = avg_intensity[c] / 8; // ~800 de pico -> 100%
            if (level > 100) level = 100;
            ui_post_audio_level_msg(level);
        }

        if (window_ok) {
            ESP_LOGI(TAG, "Ventana capturada en %lld ms", (esp_timer_get_time() - t_start) / 1000);
            ESP_LOGI(TAG, "Intensidad por trozo: [%d, %d, %d, %d]",
                 avg_intensity[0], avg_intensity[1], avg_intensity[2], avg_intensity[3]);

            vTaskPrioritySet(NULL, AI_TASK_PRIORITY_BUSY);
            ei_wrapper_classify(audio_buffer, AUDIO_BUFFER_SIZE);
            vTaskPrioritySet(NULL, AI_TASK_PRIORITY);
    }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void ai_module_start(void) {
    xTaskCreate(audio_inference_task, "ai_task", 8192, NULL, AI_TASK_PRIORITY, NULL);
}