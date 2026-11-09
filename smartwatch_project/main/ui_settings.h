#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"

// Lo que el usuario puede hacer en la pantalla de ajustes. La pantalla solo avisa de la
// accion; quien la recibe decide que significa (ui.c, con la logica de time_edit).
typedef enum {
    SETTINGS_INPUT_SELECT_HOUR,     // toco la zona de la hora
    SETTINGS_INPUT_SELECT_MINUTE,   // toco la zona de los minutos
    SETTINGS_INPUT_INCREMENT,       // toco "+" (o lo mantiene pulsado)
    SETTINGS_INPUT_DECREMENT,       // toco "-" (o lo mantiene pulsado)
} settings_input_t;

typedef void (*settings_input_cb_t)(settings_input_t input);

// Campo resaltado en pantalla (el que "+" y "-" van a cambiar).
typedef enum {
    SETTINGS_FIELD_NONE,
    SETTINGS_FIELD_HOUR,
    SETTINGS_FIELD_MINUTE,
} settings_field_t;

// Crea la pantalla de ajustes (una pantalla aparte de la principal) y la devuelve.
// No la muestra: ui.c decide cuando cargarla. on_input recibe cada accion del usuario;
// se llama en el hilo de LVGL, con el lock tomado.
lv_obj_t *ui_settings_create(settings_input_cb_t on_input);

// Muestra la hora y la fecha que se estan editando y resalta el campo seleccionado.
// time es "H:MM" (se parte en el ':' para dar a la hora y a los minutos su propia zona);
// los textos ya vienen formateados (ver clock_format_full). Solo se llama desde el hilo de
// LVGL, con el lock tomado.
void ui_settings_show_time(const char *time, const char *ampm, const char *date,
                           settings_field_t selected);

#endif // UI_SETTINGS_H
