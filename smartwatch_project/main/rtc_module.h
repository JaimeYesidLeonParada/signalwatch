#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c_master.h"

// Hora tal como la guarda el chip. dow: 0=domingo..6=sabado; month: 1..12; year: 0..99.
typedef struct {
    uint8_t sec, min, hr;
    uint8_t day, dow, month, year;
} rtc_time_t;

// Aviso periodico de la hora actual. Si valid es false no se pudo leer el chip y el
// resto de los argumentos no tiene significado.
typedef void (*rtc_clock_cb_t)(int hour, int min, int dow, int day, int month, bool valid);

// Registra el reloj en el bus I2C que ya existe
void rtc_init(i2c_master_bus_handle_t bus_handle);

// Lee la hora actual del chip. Devuelve false si falla la lectura I2C o si algun campo
// viene fuera de rango (hr 0-23, min 0-59, dow 0-6, day 1-31, month 1-12).
bool rtc_get_time(rtc_time_t *out);

void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hr, uint8_t min, uint8_t sec);

// Lanza la tarea que lee la hora cada minuto y la entrega a on_clock.
void rtc_task_start(rtc_clock_cb_t on_clock);

#endif // RTC_MODULE_H
