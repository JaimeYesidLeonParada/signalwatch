#ifndef UI_NOTIFICATION_H
#define UI_NOTIFICATION_H

#include "lvgl.h"

// Las dos comparten el mismo label en la pantalla scr; la ultima llamada es la que se ve.
// Solo se llaman desde ui_task, con el lock de LVGL tomado.
void ui_notification_show_text(lv_obj_t *scr, const char *text);
void ui_notification_show_inference(lv_obj_t *scr, const char *label, float confidence);

#endif // UI_NOTIFICATION_H
