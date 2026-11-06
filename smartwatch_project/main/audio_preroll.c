#include "audio_preroll.h"
#include <string.h>

#define CHUNK_BYTES (AUDIO_CHUNK_SAMPLES * sizeof(int16_t))

void audio_preroll_push(audio_preroll_t *preroll, const int16_t *chunk)
{
    memcpy(preroll->chunks[preroll->write_idx], chunk, CHUNK_BYTES);
    preroll->write_idx = (preroll->write_idx + 1) % AUDIO_PREROLL_CHUNKS;
}

void audio_preroll_copy_out(const audio_preroll_t *preroll, int16_t *dst)
{
    for (int i = 0; i < AUDIO_PREROLL_CHUNKS; i++) {
        int ring_pos = (preroll->write_idx + i) % AUDIO_PREROLL_CHUNKS;   // del mas viejo al mas nuevo
        memcpy(dst + (i * AUDIO_CHUNK_SAMPLES), preroll->chunks[ring_pos], CHUNK_BYTES);
    }
}
