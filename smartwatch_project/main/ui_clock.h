#ifndef UI_CLOCK_H
#define UI_CLOCK_H

#include <stdbool.h>
#include "lvgl.h"

// Hora en 12 h y fecha en la pantalla scr. dow: 0=domingo. Si valid es false se
// muestra 12:00 y sin fecha. Solo se llama desde ui_task, con el lock de LVGL tomado.
void ui_clock_update(lv_obj_t *scr, int hour, int min, int dow, int day, int month, bool valid);

#endif // UI_CLOCK_H
