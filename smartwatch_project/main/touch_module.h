#ifndef TOUCH_MODULE_H
#define TOUCH_MODULE_H

#include "driver/i2c_master.h"

// Inicializa el driver CST816 y lo enlaza directamente con LVGL
void touch_init(i2c_master_bus_handle_t bus_handle);

#endif // TOUCH_MODULE_H