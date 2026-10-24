#ifndef UI_H
#define UI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_display_init(void);

// Misma firma que rtc_clock_cb_t. dow: 0=domingo; si valid es false se muestra la hora en 12:00 y sin fecha.
void ui_post_clock_msg(int hour, int min, int dow, int day, int month, bool valid);
void ui_post_battery_msg(int bat_pct, bool is_charging);
void ui_post_inference_msg(const char *label, float confidence);
void ui_post_audio_level_msg(int level_pct);
void ui_post_alert_icon_msg(const char *label, int level); 
void ui_post_notification_msg(const char *text);

#ifdef __cplusplus
}
#endif

#endif // UI_H