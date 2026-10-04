#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_display_init(void);

void ui_post_clock_msg(const char *time_str, const char *date_str);
void ui_post_battery_msg(int bat_pct, bool is_charging);
void ui_post_inference_msg(const char *label, float confidence);
void ui_post_audio_level_msg(int level_pct);
void ui_post_alert_icon_msg(const char *label, int level); // level: 0=NONE, 1=LOW, 2=MEDIUM, 3=HIGH

#ifdef __cplusplus
}
#endif

#endif // UI_H