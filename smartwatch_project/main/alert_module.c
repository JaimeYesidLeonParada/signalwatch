#include "alert_module.h"
#include <string.h>
#include <stdbool.h>

#define ALERT_MAX_LABELS     8
#define ALERT_WINDOW_SIZE    3

#define ALERT_THRESHOLD_LOW     0.5f
#define ALERT_THRESHOLD_MEDIUM  0.6f
#define ALERT_THRESHOLD_HIGH    0.7f

typedef struct {
    char label[32];
    bool in_use;
    float buffer[ALERT_WINDOW_SIZE];
    uint32_t time_ms[ALERT_WINDOW_SIZE];   // cuando llego cada muestra
    int head;
    int count;
} alert_window_t;

static alert_window_t windows[ALERT_MAX_LABELS];

void alert_module_init(void)
{
    memset(windows, 0, sizeof(windows));
}

static alert_window_t *find_or_register(const char *label)
{
    for (int i = 0; i < ALERT_MAX_LABELS; i++) {
        if (windows[i].in_use && strncmp(windows[i].label, label, sizeof(windows[i].label) - 1) == 0) {
            return &windows[i];
        }
    }
    for (int i = 0; i < ALERT_MAX_LABELS; i++) {
        if (!windows[i].in_use) {
            windows[i].in_use = true;
            strncpy(windows[i].label, label, sizeof(windows[i].label) - 1);
            return &windows[i];
        }
    }
    return NULL; // se llego al maximo de clases registradas
}

static alert_level_t level_from_average(float avg)
{
    if (avg >= ALERT_THRESHOLD_HIGH)   return ALERT_HIGH;
    if (avg >= ALERT_THRESHOLD_MEDIUM) return ALERT_MEDIUM;
    if (avg >= ALERT_THRESHOLD_LOW)    return ALERT_LOW;
    return ALERT_NONE;
}

alert_level_t alert_module_add_sample(const char *label, float confidence, uint32_t now_ms)
{
    alert_window_t *w = find_or_register(label);
    if (w == NULL) return ALERT_NONE;

    w->buffer[w->head] = confidence;
    w->time_ms[w->head] = now_ms;
    w->head = (w->head + 1) % ALERT_WINDOW_SIZE;
    if (w->count < ALERT_WINDOW_SIZE) w->count++;

    // Solo cuentan las muestras recientes: tras un rato sin sonido, lo que se oyo antes ya no
    // describe lo que pasa ahora. La resta sin signo es correcta aunque now_ms haya dado la
    // vuelta. La muestra nueva (edad 0) siempre cuenta, asi que n >= 1.
    float sum = 0.0f;
    int n = 0;
    for (int i = 0; i < w->count; i++) {
        if ((uint32_t)(now_ms - w->time_ms[i]) <= ALERT_SAMPLE_MAX_AGE_MS) {
            sum += w->buffer[i];
            n++;
        }
    }

    return level_from_average(sum / n);
}