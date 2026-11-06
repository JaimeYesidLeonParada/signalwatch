#ifndef AUDIO_LEVEL_H
#define AUDIO_LEVEL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_LEVEL_DIVISOR  20    // promedio del valor absoluto / 20 = nivel
#define AUDIO_LEVEL_MAX      100

// Nivel (0-100) de un trozo de audio: promedio del valor absoluto de las muestras entre
// AUDIO_LEVEL_DIVISOR, recortado a AUDIO_LEVEL_MAX. Sin muestras devuelve 0.
int audio_level_from_chunk(const int16_t *samples, size_t count);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_LEVEL_H
