#include "wrist_gesture.h"

// Para no desbordar el contador si el brazo lleva horas caido.
#define SINCE_AWAY_CAP_MS 60000u

void wrist_gesture_init(wrist_gesture_t *g)
{
    g->armed = false;
    g->since_away_ms = SINCE_AWAY_CAP_MS;
    g->hold_ms = 0;
}

static bool is_steady(int ax, int ay, int az)
{
    int64_t sq = (int64_t)ax * ax + (int64_t)ay * ay + (int64_t)az * az;
    return sq >= (int64_t)WRIST_STEADY_MIN_MG * WRIST_STEADY_MIN_MG &&
           sq <= (int64_t)WRIST_STEADY_MAX_MG * WRIST_STEADY_MAX_MG;
}

bool wrist_gesture_feed(wrist_gesture_t *g, int ax_mg, int ay_mg, int az_mg)
{
    if (az_mg <= WRIST_AWAY_MAX_MG) {
        g->armed = true;
        g->since_away_ms = 0;
        g->hold_ms = 0;
        return false;
    }

    if (g->since_away_ms < SINCE_AWAY_CAP_MS) {
        g->since_away_ms += WRIST_GESTURE_SAMPLE_PERIOD_MS;
    }

    if (!g->armed || az_mg < WRIST_LOOK_MIN_MG || !is_steady(ax_mg, ay_mg, az_mg)) {
        g->hold_ms = 0;
        return false;
    }

    g->hold_ms += WRIST_GESTURE_SAMPLE_PERIOD_MS;
    if (g->hold_ms >= WRIST_HOLD_MS && g->since_away_ms <= WRIST_RAISE_WINDOW_MS) {
        g->armed = false;
        g->hold_ms = 0;
        return true;
    }
    return false;
}
