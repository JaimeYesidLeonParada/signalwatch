#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_display_init(void);

void ui_post_clock_msg(const char *time_str);
void ui_post_battery_msg(int bat_pct);
void ui_post_inference_msg(const char *label, float confidence);
void ui_post_audio_level_msg(int level_pct);

#ifdef __cplusplus
}
#endif

#endif // UI_H