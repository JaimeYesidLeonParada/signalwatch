#include "ei_wrapper.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "esp_log.h"
#include "esp_timer.h" 
#include <stdarg.h>
#include <stdlib.h>

static const char *TAG = "edge_ai";

// ============================================================================
// NATIVE PORTING: Connecting Edge Impulse with ESP32-C6 hardware
// ============================================================================

void ei_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void ei_printf_float(float f) {
    printf("%f", f);
}

void *ei_malloc(size_t size) {
    return malloc(size);
}

void *ei_calloc(size_t nitems, size_t size) {
    return calloc(nitems, size);
}

void ei_free(void *ptr) {
    free(ptr);
}

// Time and cancellation functions
uint64_t ei_read_timer_us() {
    return (uint64_t)esp_timer_get_time();
}

uint64_t ei_read_timer_ms() {
    return (uint64_t)(esp_timer_get_time() / 1000);
}

// FIX: Changed return type from bool to EI_IMPULSE_ERROR
EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK; // Never cancel the inference mid-flight
}

// ============================================================================
// WRAPPER FUNCTIONS
// ============================================================================

void ei_wrapper_init(void) {
    ESP_LOGI(TAG, "=========================================");
    ESP_LOGI(TAG, "Waking up Artificial Intelligence Engine...");
    ESP_LOGI(TAG, "Edge Impulse Project: %s", EI_CLASSIFIER_PROJECT_NAME);
    ESP_LOGI(TAG, "Expected sample rate: %d Hz", EI_CLASSIFIER_FREQUENCY);
    ESP_LOGI(TAG, "Audio frame size: %d samples", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ESP_LOGI(TAG, "Number of classes to detect: %d", EI_CLASSIFIER_LABEL_COUNT);
    ESP_LOGI(TAG, "=========================================");
}

void ei_wrapper_classify(int16_t *audio_buffer, int buffer_size) {
    // Left empty for now. One step at a time.
}