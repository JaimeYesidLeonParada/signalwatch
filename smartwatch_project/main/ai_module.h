#ifndef AI_MODULE_H
#define AI_MODULE_H

#include "esp_codec_dev.h"
#include "ei_wrapper.h"

// A quien avisar de lo que captura y clasifica ai_module. Un callback en NULL se ignora.
typedef struct {
    // Nivel de audio (0-100) de cada chunk, desde la tarea de captura.
    void (*on_audio_level)(int level_pct);
    // Resultado de cada clasificacion exitosa, desde la tarea de clasificacion.
    void (*on_classification)(const ei_wrapper_result_t *result);
    // El audio (n muestras de 16 bits a 16 kHz) que se acaba de clasificar, con su resultado,
    // desde la tarea de clasificacion. samples solo es valido durante la llamada. Sirve para
    // diagnostico (estadisticas, volcado del audio); no se usa en el funcionamiento normal.
    void (*on_window)(const int16_t *samples, int n, const ei_wrapper_result_t *result);
} ai_listener_t;

void ai_module_start(esp_codec_dev_handle_t mic, const ai_listener_t *listener);

#endif // AI_MODULE_H
