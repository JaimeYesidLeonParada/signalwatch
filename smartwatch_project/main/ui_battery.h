#ifndef UI_BATTERY_H
#define UI_BATTERY_H

#include <stdbool.h>
#include "lvgl.h"

// Icono de bateria (y de USB si carga) en la pantalla scr. Solo se llama desde ui_task,
// con el lock de LVGL tomado.
void ui_battery_update(lv_obj_t *scr, int bat_pct, bool is_charging);

#endif // UI_BATTERY_H
