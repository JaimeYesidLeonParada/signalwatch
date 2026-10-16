#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ai_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_codec_dev.h"
#include "esp_timer.h"
#include "ei_wrapper.h"

#define AUDIO_BUFFER_SIZE   16000   // 1 s @ 16 kHz (lo que exige el modelo)
#define CHUNK_SAMPLES       1000
#define CHUNKS_PER_WINDOW   (AUDIO_BUFFER_SIZE / CHUNK_SAMPLES)

#define CAPTURE_TASK_PRIORITY    5   // la mas alta: protege el margen del DMA
#define CLASSIFY_TASK_PRIORITY   0   // la mas baja (idle): nunca bloquea al resto del sistema
#define SOUND_THRESHOLD   15
#define PRE_ROLL_CHUNKS   4   // ~250 ms de contexto antes del disparo (AJUSTAR con pruebas)

static int16_t preroll_ring[PRE_ROLL_CHUNKS][CHUNK_SAMPLES];
static int preroll_write_idx = 0;
static int16_t chunk_scratch[CHUNK_SAMPLES]; // buffer de lectura reutilizable, static para no gastar stack

static const char *TAG = "ai_module";

static esp_codec_dev_handle_t mic_dev;
static ai_listener_t listener;

// Doble buffer: mientras uno se clasifica, el otro se esta llenando
static int16_t *audio_buffers[2];

// buffer_ready_queue: captura avisa "este indice ya esta lleno, listo para clasificar"
// buffer_free_queue:  clasificacion avisa "este indice ya quedo libre, se puede reusar"
static QueueHandle_t buffer_ready_queue;
static QueueHandle_t buffer_free_queue;

static void audio_capture_task(void *arg) {
    const size_t chunk_bytes = CHUNK_SAMPLES * sizeof(int16_t);

    bool filling = false;      // false = buscando un disparo, true = completando una ventana ya disparada
    int buf_idx = -1;
    int chunks_filled = 0;
    int64_t fill_start = 0;

    while (1) {
        int err = esp_codec_dev_read(mic_dev, (void *)chunk_scratch, chunk_bytes);
        if (err != ESP_CODEC_DEV_OK) {
            ESP_LOGE(TAG, "Error leyendo microfono: %d", err);
            continue;
        }

        int64_t sum = 0;
        for (int i = 0; i < CHUNK_SAMPLES; i++) sum += abs(chunk_scratch[i]);

        int level = (int)((sum / CHUNK_SAMPLES) / 20);
        if (level > 100) level = 100;
        if (listener.on_audio_level) listener.on_audio_level(level);

        if (!filling) {
            memcpy(preroll_ring[preroll_write_idx], chunk_scratch, chunk_bytes);
            preroll_write_idx = (preroll_write_idx + 1) % PRE_ROLL_CHUNKS;

            if (level >= SOUND_THRESHOLD) {
                xQueueReceive(buffer_free_queue, &buf_idx, portMAX_DELAY);
                int16_t *dst = audio_buffers[buf_idx];

                for (int i = 0; i < PRE_ROLL_CHUNKS; i++) {
                    int ring_pos = (preroll_write_idx + i) % PRE_ROLL_CHUNKS; // del mas viejo al mas nuevo
                    memcpy(dst + (i * CHUNK_SAMPLES), preroll_ring[ring_pos], chunk_bytes);
                }

                chunks_filled = PRE_ROLL_CHUNKS;
                fill_start = esp_timer_get_time();
                filling = true;
                ESP_LOGI(TAG, "Disparo (nivel %d) -> buffer %d", level, buf_idx);
            }
        } else {
            int16_t *dst = audio_buffers[buf_idx];
            memcpy(dst + (chunks_filled * CHUNK_SAMPLES), chunk_scratch, chunk_bytes);
            chunks_filled++;

            if (chunks_filled >= CHUNKS_PER_WINDOW) {
                ESP_LOGI(TAG, "Buffer %d completo en %lld ms", buf_idx, (esp_timer_get_time() - fill_start) / 1000);
                xQueueSend(buffer_ready_queue, &buf_idx, portMAX_DELAY);
                filling = false;
            }
        }
    }
}

static void audio_classify_task(void *arg) {
    while (1) {
        int buf_idx;
        xQueueReceive(buffer_ready_queue, &buf_idx, portMAX_DELAY);

        ei_wrapper_result_t result;
        if (ei_wrapper_classify(audio_buffers[buf_idx], AUDIO_BUFFER_SIZE, &result)) {
            if (listener.on_classification) listener.on_classification(&result);
        }

        xQueueSend(buffer_free_queue, &buf_idx, portMAX_DELAY);
    }
}

void ai_module_start(esp_codec_dev_handle_t mic, const ai_listener_t *l) {
    mic_dev = mic;
    if (l != NULL) listener = *l;

    audio_buffers[0] = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
    audio_buffers[1] = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));

    if (audio_buffers[0] == NULL || audio_buffers[1] == NULL) {
        ESP_LOGE(TAG, "Memoria insuficiente para los buffers de audio!");
        return;
    }

    buffer_ready_queue = xQueueCreate(2, sizeof(int));
    buffer_free_queue  = xQueueCreate(2, sizeof(int));

    // Al arrancar, los dos buffers estan libres
    int idx0 = 0, idx1 = 1;
    xQueueSend(buffer_free_queue, &idx0, 0);
    xQueueSend(buffer_free_queue, &idx1, 0);

    xTaskCreate(audio_capture_task, "capture_task", 6144, NULL, CAPTURE_TASK_PRIORITY, NULL);
    xTaskCreate(audio_classify_task, "classify_task", 8192, NULL, CLASSIFY_TASK_PRIORITY, NULL);
}