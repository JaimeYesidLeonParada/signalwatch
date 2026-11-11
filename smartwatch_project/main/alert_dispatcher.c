#include "alert_dispatcher.h"
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "alert_module.h"
#include "alert_catalog.h"

#define LOUD_SOUND_THRESHOLD   90

static const char *TAG = "alert_dispatcher";

static const char *const ALERT_LEVEL_NAMES[] = { "NONE", "LOW", "MEDIUM", "HIGH" };

static alert_outputs_t outputs;
static bool was_loud = false;

static const char *morse_for_alert_label(const char *label)
{
    int idx = alert_catalog_index_of(label);
    return (idx >= 0) ? alert_catalog_get((size_t)idx)->morse : NULL;
}

void alert_dispatcher_init(const alert_outputs_t *out)
{
    if (out != NULL) outputs = *out;
    else             memset(&outputs, 0, sizeof(outputs));

    alert_module_init();
    was_loud = false;
}

void alert_dispatcher_on_audio_level(int level_pct)
{
    if (outputs.audio_level) outputs.audio_level(level_pct);

    bool is_loud = (level_pct >= LOUD_SOUND_THRESHOLD);
    if (is_loud && !was_loud) {
        if (outputs.vibrate_morse) outputs.vibrate_morse("I");
        if (outputs.notification)  outputs.notification("Ruido fuerte cerca!");
    }
    was_loud = is_loud;
}

void alert_dispatcher_on_classification(const ei_wrapper_result_t *result)
{
    if (outputs.inference) {
        outputs.inference(result->classifications[result->best_index].label,
                          result->classifications[result->best_index].confidence);
    }

    for (int i = 0; i < result->count; i++) {
        const char *label = result->classifications[i].label;
        alert_level_t lvl = alert_module_add_sample(label, result->classifications[i].confidence,
                                                  result->timestamp_ms);
        ESP_LOGI(TAG, "    %s -> nivel post-procesado: %s", label, ALERT_LEVEL_NAMES[lvl]);
        if (outputs.alert_icon) outputs.alert_icon(label, (int)lvl);

        if (lvl == ALERT_HIGH || lvl == ALERT_MEDIUM) {
            const char *morse = morse_for_alert_label(label);
            if (morse != NULL && outputs.vibrate_morse) {
                outputs.vibrate_morse(morse);
            }
            // Solo las clases del catalogo encienden la pantalla; el ruido fuerte no.
            if (morse != NULL && outputs.wake_screen) {
                outputs.wake_screen();
            }
        }
    }
}
