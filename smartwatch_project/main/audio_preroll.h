#ifndef AUDIO_PREROLL_H
#define AUDIO_PREROLL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_CHUNK_SAMPLES   1000   // muestras por trozo leido del microfono
#define AUDIO_PREROLL_CHUNKS  4      // ~250 ms de contexto antes del disparo (AJUSTAR con pruebas)

// Guarda los ultimos AUDIO_PREROLL_CHUNKS trozos para poder entregar el audio previo al disparo.
// Puesto a cero (static o memset) esta listo para usar: los trozos que aun no llegaron son silencio.
typedef struct {
    int16_t chunks[AUDIO_PREROLL_CHUNKS][AUDIO_CHUNK_SAMPLES];
    int write_idx;
} audio_preroll_t;

// Guarda un trozo de AUDIO_CHUNK_SAMPLES muestras; si esta lleno, reemplaza al mas viejo.
void audio_preroll_push(audio_preroll_t *preroll, const int16_t *chunk);

// Copia en dst los trozos guardados, del mas viejo al mas nuevo
// (AUDIO_PREROLL_CHUNKS * AUDIO_CHUNK_SAMPLES muestras).
void audio_preroll_copy_out(const audio_preroll_t *preroll, int16_t *dst);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_PREROLL_H
