#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"

// Crea la pantalla de ajustes (una pantalla aparte de la principal) y la devuelve.
// No la muestra: ui.c decide cuando cargarla.
lv_obj_t *ui_settings_create(void);

// Muestra la hora y la fecha que se van a editar. Los textos ya vienen formateados
// (ver clock_format_full). Solo se llama desde el hilo de LVGL, con el lock tomado.
void ui_settings_show_time(const char *time, const char *ampm, const char *date);

#endif // UI_SETTINGS_H
