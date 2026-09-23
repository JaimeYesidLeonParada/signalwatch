#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include "driver/i2c_master.h"

// Registra el reloj en el bus I2C que ya existe
void rtc_init(i2c_master_bus_handle_t bus_handle);

// Lee la hora actual y la formatea en un string (ej: "14:30:05")
void rtc_get_time_str(char *time_str, size_t max_len);

void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hr, uint8_t min, uint8_t sec);

void rtc_task_start(void);

#endif // RTC_MODULE_H