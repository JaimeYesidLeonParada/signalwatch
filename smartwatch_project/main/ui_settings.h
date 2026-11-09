#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"

// Crea la pantalla de ajustes (una pantalla aparte de la principal) y la devuelve.
// No la muestra: ui.c decide cuando cargarla. Por ahora solo tiene un titulo.
lv_obj_t *ui_settings_create(void);

#endif // UI_SETTINGS_H
