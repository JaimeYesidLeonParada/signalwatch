#include "audio_level.h"

int audio_level_from_chunk(const int16_t *samples, size_t count)
{
    if (count == 0) return 0;

    int64_t sum = 0;
    for (size_t i = 0; i < count; i++) {
        int sample = samples[i];
        sum += (sample < 0) ? -sample : sample;   // en int, -(-32768) no desborda
    }

    int level = (int)((sum / (int64_t)count) / AUDIO_LEVEL_DIVISOR);
    if (level > AUDIO_LEVEL_MAX) level = AUDIO_LEVEL_MAX;
    return level;
}
