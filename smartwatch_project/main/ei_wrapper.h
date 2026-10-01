#ifndef EI_WRAPPER_H
#define EI_WRAPPER_H

#include <stdint.h>  // <-- Librería requerida para int16_t
#include <stdbool.h> // <-- Para el bool de retorno

#ifdef __cplusplus
extern "C" {
#endif

#define EI_WRAPPER_MAX_LABELS 8

typedef struct {
    char label[32];
    float confidence;
} ei_wrapper_classification_t;

typedef struct {
    ei_wrapper_classification_t classifications[EI_WRAPPER_MAX_LABELS];
    int count;
    int best_index;
    int timing_ms;
} ei_wrapper_result_t;

// Inicia el modelo (reserva de memoria, etc)
void ei_wrapper_init(void);

// Le pasamos el buffer de audio para que lo clasifique.
// Devuelve true si la clasificacion fue exitosa, y llena out_result.
bool ei_wrapper_classify(int16_t *audio_buffer, int buffer_size, ei_wrapper_result_t *out_result);

#ifdef __cplusplus
}
#endif

#endif // EI_WRAPPER_H