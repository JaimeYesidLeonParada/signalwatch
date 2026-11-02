#ifndef TOUCH_MODULE_H
#define TOUCH_MODULE_H

#include "driver/i2c_master.h"
#include "lvgl.h"

// Inicializa el driver CST816 y lo enlaza con LVGL en el display indicado, que ya
// debe estar inicializado (ver display_hw_init).
void touch_init(i2c_master_bus_handle_t bus_handle, lv_disp_t *disp);

#endif // TOUCH_MODULE_H