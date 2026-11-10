#ifndef UI_SLEEP_H
#define UI_SLEEP_H

#include "lvgl.h"
#include "display_power.h"

// Apagado automatico de la pantalla. Une la decision (screen_policy, logica pura) con LVGL:
// mide la inactividad del touch, apaga y enciende la pantalla por display_power_t y hace que
// el toque que la despierta no pulse nada de lo que hay debajo.
// Todo se llama desde el hilo de LVGL, con el lock tomado (salvo ui_sleep_set_power).

// Se llama justo despues de apagar la pantalla (por ejemplo para volver a la pantalla principal).
typedef void (*ui_sleep_cb_t)(void);

// Empieza a vigilar la inactividad de display. on_sleep puede ser NULL.
void ui_sleep_create(lv_display_t *display, ui_sleep_cb_t on_sleep);

// Conecta como se enciende y apaga la pantalla. Mientras no se conecte, nunca se apaga.
void ui_sleep_set_power(display_power_t power);

// Hubo actividad que no es un toque (boton, gesto, alerta): enciende la pantalla si estaba
// apagada y reinicia la cuenta. Los toques los detecta este modulo por su cuenta.
void ui_sleep_activity(void);

#endif // UI_SLEEP_H
