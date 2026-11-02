#ifndef BATTERY_MODULE_H
#define BATTERY_MODULE_H

#include <stdbool.h>

// Aviso periodico del estado de la bateria: porcentaje (0-100) y si esta cargando por USB.
typedef void (*battery_status_cb_t)(int pct, bool is_charging);

// Habilita la alimentacion desde la bateria (GPIO de la placa). Debe ser lo primero
// que corra al arrancar.
void battery_power_enable(void);

// Configura el ADC y la calibración para leer la batería.
void battery_init(void);

// Lanza la tarea que entrega el estado a on_status cada segundo.
void battery_start(battery_status_cb_t on_status);
int battery_get_percentage(void);

#endif // BATTERY_MODULE_H
