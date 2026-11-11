#include "audio_stats.h"
#include <math.h>

void audio_stats_compute(const int16_t *samples, int n, audio_stats_t *out)
{
    out->peak = 0;
    out->rms = 0;
    out->clipped = 0;
    if (samples == NULL || n <= 0) {
        return;
    }

    double sum_sq = 0.0;
    for (int i = 0; i < n; i++) {
        int v = samples[i];
        int a = (v < 0) ? -v : v;   // -32768 da 32768, sin desbordar el int
        if (a > out->peak) out->peak = a;
        if (a >= AUDIO_CLIP_LEVEL) out->clipped++;
        sum_sq += (double)v * v;
    }
    out->rms = (int)sqrt(sum_sq / n);
}
