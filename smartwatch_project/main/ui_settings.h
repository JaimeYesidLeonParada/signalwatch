#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"

// Lo que el usuario puede hacer en la pantalla de ajustes. La pantalla solo avisa de la
// accion; quien la recibe decide que significa (ui.c, con la logica de time_edit).
typedef enum {
    SETTINGS_INPUT_SELECT_HOUR,     // toco la zona de la hora
    SETTINGS_INPUT_SELECT_MINUTE,   // toco la zona de los minutos
    SETTINGS_INPUT_SELECT_DAY,      // toco la zona del dia
    SETTINGS_INPUT_SELECT_MONTH,    // toco la zona del mes
    SETTINGS_INPUT_SELECT_YEAR,     // toco la zona del anio
    SETTINGS_INPUT_INCREMENT,       // toco "+" (o lo mantiene pulsado)
    SETTINGS_INPUT_DECREMENT,       // toco "-" (o lo mantiene pulsado)
    SETTINGS_INPUT_SAVE,            // toco el check de guardar
} settings_input_t;

typedef void (*settings_input_cb_t)(settings_input_t input);

// Campo resaltado en pantalla (el que "+" y "-" van a cambiar).
typedef enum {
    SETTINGS_FIELD_HOUR,
    SETTINGS_FIELD_MINUTE,
    SETTINGS_FIELD_DAY,
    SETTINGS_FIELD_MONTH,
    SETTINGS_FIELD_YEAR,
    SETTINGS_FIELD_COUNT
} settings_field_t;

// Todo lo que la pantalla muestra. Los textos ya vienen formateados (ver clock_format_full).
typedef struct {
    const char *time;       // "H:MM"; se parte en el ':' para dar a hora y minutos su propia zona
    const char *ampm;       // "AM" o "PM"
    const char *day;        // "8"
    const char *month;      // "Nov"
    const char *year;       // "2026"
    settings_field_t selected;
} settings_view_t;

// Crea la pantalla de ajustes (una pantalla aparte de la principal) y la devuelve.
// No la muestra: ui.c decide cuando cargarla. on_input recibe cada accion del usuario;
// se llama en el hilo de LVGL, con el lock tomado.
lv_obj_t *ui_settings_create(settings_input_cb_t on_input);

// Muestra la hora y la fecha que se estan editando y resalta el campo seleccionado.
// Solo se llama desde el hilo de LVGL, con el lock tomado.
void ui_settings_show(const settings_view_t *view);

// Muestra un aviso breve (por ejemplo "No se pudo guardar") que se borra solo a los pocos
// segundos, o antes si se vuelve a dibujar la pantalla. Solo desde el hilo de LVGL, con el lock.
void ui_settings_show_message(const char *text);

#endif // UI_SETTINGS_H
