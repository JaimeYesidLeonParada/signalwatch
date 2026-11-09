#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "lvgl.h"
#include "time_source.h"

#ifdef __cplusplus
extern "C" {
#endif

// Crea los widgets, la cola y ui_task sobre un display ya inicializado (ver display_hw_init).
void ui_start(lv_disp_t *display);

// Conecta de donde lee la hora la pantalla de ajustes (la llama app_init.c una vez que el RTC existe).
// Mientras no se conecte, los ajustes muestran la hora por defecto.
void ui_set_time_source(time_source_t source);

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