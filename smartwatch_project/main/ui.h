#ifndef UI_H
#define UI_H

void ui_display_init(void);
void ui_post_clock_msg(const char *time_str);
void ui_post_battery_msg(int bat_mv, int bat_pct);
void ui_post_inference_msg(const char *label, float confidence);

#endif