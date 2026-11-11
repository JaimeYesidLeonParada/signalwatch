#ifndef AUDIO_STATS_H
#define AUDIO_STATS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Muestras con este valor absoluto o mas se consideran saturadas (recortadas): el microfono o
// el codec llegaron al tope y la forma de onda se deformo.
#define AUDIO_CLIP_LEVEL 32700

typedef struct {
    int peak;      // mayor valor absoluto (0..32768)
    int rms;       // valor eficaz: la "potencia" media de la senal
    int clipped;   // cuantas muestras estan saturadas
} audio_stats_t;

// Calcula las estadisticas de n muestras de 16 bits. Con n <= 0 todo es 0.
void audio_stats_compute(const int16_t *samples, int n, audio_stats_t *out);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_STATS_H
