#include "alert_dispatcher.h"
#include <stdbool.h>
#include "esp_log.h"
#include "alert_module.h"
#include "alert_catalog.h"
#include "vibrator_module.h"
#include "ui.h"

#define LOUD_SOUND_THRESHOLD   90

static const char *TAG = "alert_dispatcher";

static const char *const ALERT_LEVEL_NAMES[] = { "NONE", "LOW", "MEDIUM", "HIGH" };

static bool was_loud = false;

static const char *morse_for_alert_label(const char *label)
{
    int idx = alert_catalog_index_of(label);
    return (idx >= 0) ? alert_catalog_get((size_t)idx)->morse : NULL;
}

void alert_dispatcher_init(void)
{
    alert_module_init();
    was_loud = false;
}

void alert_dispatcher_on_audio_level(int level_pct)
{
    ui_post_audio_level_msg(level_pct);

    bool is_loud = (level_pct >= LOUD_SOUND_THRESHOLD);
    if (is_loud && !was_loud) {
        vibration_morse_play("I");
        ui_post_notification_msg("Ruido fuerte cerca!");
    }
    was_loud = is_loud;
}

void alert_dispatcher_on_classification(const ei_wrapper_result_t *result)
{
    ui_post_inference_msg(result->classifications[result->best_index].label,
                          result->classifications[result->best_index].confidence);

    for (int i = 0; i < result->count; i++) {
        const char *label = result->classifications[i].label;
        alert_level_t lvl = alert_module_add_sample(label, result->classifications[i].confidence);
        ESP_LOGI(TAG, "    %s -> nivel post-procesado: %s", label, ALERT_LEVEL_NAMES[lvl]);
        ui_post_alert_icon_msg(label, (int)lvl);

        if (lvl == ALERT_HIGH || lvl == ALERT_MEDIUM) {
            const char *morse = morse_for_alert_label(label);
            if (morse != NULL) {
                vibration_morse_play(morse);
            }
        }
    }
}
