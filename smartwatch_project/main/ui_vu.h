#ifndef UI_VU_H
#define UI_VU_H

#include "lvgl.h"

// Crea el icono de volumen en la pantalla scr y lo devuelve (sirve de ancla a otros widgets).
lv_obj_t *ui_vu_create(lv_obj_t *scr);

// Solo se llama desde ui_task, con el lock de LVGL tomado.
void ui_vu_update(int level_pct);

#endif // UI_VU_H
