#ifndef DISPLAY_HW_H
#define DISPLAY_HW_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa el hardware de la pantalla (backlight, SPI, panel ST7789) y LVGL.
// Devuelve el display de LVGL; no dibuja nada.
lv_disp_t *display_hw_init(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_HW_H
