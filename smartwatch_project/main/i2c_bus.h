#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "driver/i2c_master.h"

// Crea el bus I2C compartido por el codec, el RTC y el touch.
i2c_master_bus_handle_t i2c_bus_init(void);

#endif // I2C_BUS_H
