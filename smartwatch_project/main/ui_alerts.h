#ifndef UI_ALERTS_H
#define UI_ALERTS_H

#include "lvgl.h"

// Crea un icono por clase del catalogo, en fila a la derecha de anchor.
void ui_alerts_create(lv_obj_t *scr, lv_obj_t *anchor);

// Cambia el estilo del icono de la clase label y, si cambio el nivel, el texto de aviso.
// Solo se llama desde ui_task, con el lock de LVGL tomado.
void ui_alerts_update(lv_obj_t *scr, const char *label, int level);

#endif // UI_ALERTS_H
