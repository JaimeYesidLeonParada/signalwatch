#include "wrist_wake.h"
#include "wrist_gesture.h"
#include "esp_log.h"

static const char *TAG = "wrist_wake";

static wrist_gesture_t gesture;
static wrist_wake_cb_t gesture_cb;

void wrist_wake_init(wrist_wake_cb_t on_gesture)
{
    gesture_cb = on_gesture;
    wrist_gesture_init(&gesture);
}

void wrist_wake_reset(void)
{
    wrist_gesture_init(&gesture);
}

void wrist_wake_on_sample(int ax_mg, int ay_mg, int az_mg)
{
    if (wrist_gesture_feed(&gesture, ax_mg, ay_mg, az_mg)) {
        ESP_LOGI(TAG, "gesto de muneca detectado");
        if (gesture_cb != NULL) {
            gesture_cb();
        }
    }
}
