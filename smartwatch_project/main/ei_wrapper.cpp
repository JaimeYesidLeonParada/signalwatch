#include "ei_wrapper.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "esp_log.h"
#include "esp_timer.h" 
#include <stdarg.h>
#include <stdlib.h>

static const char *TAG = "edge_ai";

// Puntero global temporal para compartir el audio con la función de Edge Impulse
static int16_t *current_audio_buffer = NULL;

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
// Callback que Edge Impulse llama para pedir los datos de audio
int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        // Convertimos el audio de 16 bits (entero) a float
        out_ptr[i] = (float)current_audio_buffer[offset + i];
    }
    return 0;
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
    if (buffer_size < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ESP_LOGE(TAG, "El buffer de audio es muy pequeno. Se esperaban %d muestras.", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
        return;
    }

    // Asignamos el buffer actual al puntero global
    current_audio_buffer = audio_buffer;

    // Configuramos la estructura de la senal para Edge Impulse
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;

    // Ejecutamos la inferencia
    ei_impulse_result_t result = { 0 };
    ESP_LOGI(TAG, "Ejecutando inferencia...");
    
    // El 'false' al final desactiva el modo debug detallado para ahorrar tiempo
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
    
    if (err != EI_IMPULSE_OK) {
        ESP_LOGE(TAG, "Fallo al ejecutar el clasificador (%d)", err);
        return;
    }

    // Imprimimos las predicciones!
    ESP_LOGI(TAG, "Predicciones (Tardo %d ms):", result.timing.classification);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ESP_LOGI(TAG, "    %s: %.5f", result.classification[ix].label, result.classification[ix].value);
    }
}