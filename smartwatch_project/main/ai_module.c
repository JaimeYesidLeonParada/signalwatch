#include "ai_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "ei_wrapper.h"
#include <stdlib.h>

#define AUDIO_BUFFER_SIZE 16000

static const char *TAG = "ai_module";

// Import from the audio BSP
extern esp_codec_dev_handle_t input_dev;

static void audio_inference_task(void *arg) {
    ESP_LOGI(TAG, "Allocating 32KB of memory for the audio buffer...");
    
    // Allocate memory on the heap to prevent Stack Overflow
    int16_t *audio_buffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
    
    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "Insufficient memory for audio buffer!");
        vTaskDelete(NULL);
    }

    size_t bytes_to_read = AUDIO_BUFFER_SIZE * sizeof(int16_t);

    while(1) {
        ESP_LOGI(TAG, "Listening (1 second)... Say a keyword!");

        // Read 1 second of audio from the ES8311 driver
        int err = esp_codec_dev_read(input_dev, (void *)audio_buffer, bytes_to_read);
        
        if (err == ESP_CODEC_DEV_OK) {
            // Pass the buffer to the neural network
            ei_wrapper_classify(audio_buffer, AUDIO_BUFFER_SIZE);
        } else {
            ESP_LOGE(TAG, "Error reading microphone: %d", err);
        }

        // Small delay to yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void ai_module_start(void) {
    // Launch the AI task (8KB stack is safe for TFLite Micro)
    xTaskCreate(audio_inference_task, "ai_task", 8192, NULL, 5, NULL);
}