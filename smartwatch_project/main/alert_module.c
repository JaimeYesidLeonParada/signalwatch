#include "alert_module.h"
#include <string.h>
#include <stdbool.h>

#define ALERT_MAX_LABELS     8
#define ALERT_WINDOW_SIZE    4

#define ALERT_THRESHOLD_LOW     0.6f
#define ALERT_THRESHOLD_MEDIUM  0.7f
#define ALERT_THRESHOLD_HIGH    0.8f

typedef struct {
    char label[32];
    bool in_use;
    float buffer[ALERT_WINDOW_SIZE];
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
        if (windows[i].in_use && strcmp(windows[i].label, label) == 0) {
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

alert_level_t alert_module_add_sample(const char *label, float confidence)
{
    alert_window_t *w = find_or_register(label);
    if (w == NULL) return ALERT_NONE;

    w->buffer[w->head] = confidence;
    w->head = (w->head + 1) % ALERT_WINDOW_SIZE;
    if (w->count < ALERT_WINDOW_SIZE) w->count++;

    float sum = 0.0f;
    for (int i = 0; i < w->count; i++) sum += w->buffer[i];
    float avg = sum / w->count;

    return level_from_average(avg);
}