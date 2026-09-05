#ifndef VUVOMETRO_H
#define VUVOMETRO_H

#include "driver/i2c_master.h"

void vuvometro_init(i2c_master_bus_handle_t bus_handle);
void vuvometro_start(void);

#endif // VUVOMETRO_H