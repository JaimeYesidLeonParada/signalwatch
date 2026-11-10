#ifndef DISPLAY_HW_H
#define DISPLAY_HW_H

#include <stdbool.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa el hardware de la pantalla (backlight, SPI, panel ST7789) y LVGL.
// Devuelve el display de LVGL; no dibuja nada.
lv_disp_t *display_hw_init(void);

// Enciende o apaga la luz de fondo (lo que mas consume de la pantalla). El panel y LVGL siguen
// funcionando, asi que al encender la imagen ya esta al dia. Cumple display_power_t.
void display_hw_set_backlight(bool on);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_HW_H
