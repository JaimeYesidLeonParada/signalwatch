#include "audio_diag.h"
#include "audio_stats.h"
#include "base64_encode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "audio_diag";

#define SAMPLE_RATE_HZ      16000
#define DUMP_MAX_SAMPLES    16000                 // 1 s: lo que clasifica el modelo
#define DUMP_LINE_BYTES     96                    // multiplo de 3: cada linea se decodifica sola
#define DUMP_LINE_CHARS     BASE64_ENCODED_SIZE(DUMP_LINE_BYTES)

#if AUDIO_DIAG_DUMP
static int16_t *dump_buf;
static volatile bool dump_busy;        // true desde que se copia la ventana hasta que se imprime
static char dump_label[32];
static float dump_confidence;
static int dump_samples;
static int dump_id;
static int64_t last_dump_us;
static TaskHandle_t dump_task_handle;

static void dump_task(void *arg)
{
    char line[DUMP_LINE_CHARS];
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Todas las lineas llevan el prefijo "AD:" para separarlas del resto del log.
        printf("AD:BEGIN %d %d %d %s %.3f\n", dump_id, SAMPLE_RATE_HZ, dump_samples, dump_label,
               (double)dump_confidence);
        const uint8_t *bytes = (const uint8_t *)dump_buf;
        size_t total = (size_t)dump_samples * sizeof(int16_t);
        for (size_t off = 0; off < total; off += DUMP_LINE_BYTES) {
            size_t chunk = (total - off < DUMP_LINE_BYTES) ? (total - off) : DUMP_LINE_BYTES;
            base64_encode(bytes + off, chunk, line);
            printf("AD:%s\n", line);
            vTaskDelay(1);   // deja pasar al resto de tareas (y su log) entre linea y linea
        }
        printf("AD:END %d\n", dump_id);
        dump_busy = false;
    }
}
#endif

void audio_diag_init(void)
{
#if AUDIO_DIAG_DUMP
    dump_buf = (int16_t *)malloc(DUMP_MAX_SAMPLES * sizeof(int16_t));
    if (dump_buf == NULL) {
        ESP_LOGW(TAG, "sin memoria para el volcado de audio: solo estadisticas");
        return;
    }
    xTaskCreate(dump_task, "audio_dump", 4096, NULL, 1, &dump_task_handle);
    ESP_LOGI(TAG, "volcado de audio activo (una ventana cada %d s)", AUDIO_DIAG_DUMP_INTERVAL_MS / 1000);
#endif
}

void audio_diag_on_window(const int16_t *samples, int n, const ei_wrapper_result_t *result)
{
    audio_stats_t st;
    audio_stats_compute(samples, n, &st);

    const char *best = "?";
    float best_conf = 0.0f;
    if (result != NULL && result->count > 0) {
        best = result->classifications[result->best_index].label;
        best_conf = result->classifications[result->best_index].confidence;
    }
    ESP_LOGI(TAG, "ventana: pico=%d rms=%d saturadas=%d (%d%%) -> %s %.2f", st.peak, st.rms,
             st.clipped, n > 0 ? (st.clipped * 100) / n : 0, best, (double)best_conf);

#if AUDIO_DIAG_DUMP
    int64_t now = esp_timer_get_time();
    if (dump_buf == NULL || dump_busy || n <= 0 ||
        (now - last_dump_us) < (int64_t)AUDIO_DIAG_DUMP_INTERVAL_MS * 1000) {
        return;
    }
    last_dump_us = now;
    dump_samples = (n < DUMP_MAX_SAMPLES) ? n : DUMP_MAX_SAMPLES;
    memcpy(dump_buf, samples, (size_t)dump_samples * sizeof(int16_t));
    strncpy(dump_label, best, sizeof(dump_label) - 1);
    dump_label[sizeof(dump_label) - 1] = '\0';
    dump_confidence = best_conf;
    dump_id++;
    dump_busy = true;
    xTaskNotifyGive(dump_task_handle);
#endif
}
