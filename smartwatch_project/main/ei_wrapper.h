#ifndef EI_WRAPPER_H
#define EI_WRAPPER_H

#include <stdint.h> // <-- Librería requerida para int16_t

#ifdef __cplusplus
extern "C" {
#endif

// Inicia el modelo (reserva de memoria, etc)
void ei_wrapper_init(void);

// Le pasamos el buffer de audio para que lo clasifique
void ei_wrapper_classify(int16_t *audio_buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif // EI_WRAPPER_H