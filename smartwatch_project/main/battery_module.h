#ifndef BATTERY_MODULE_H
#define BATTERY_MODULE_H

#include <stdbool.h>

// Aviso periodico del estado de la bateria: porcentaje (0-100) y si esta cargando por USB.
typedef void (*battery_status_cb_t)(int pct, bool is_charging);

// Configura el ADC y la calibración para leer la batería, y lanza la tarea que
// entrega el estado a on_status cada segundo.
void battery_init(battery_status_cb_t on_status);
int battery_get_percentage(void);

#endif // BATTERY_MODULE_H
