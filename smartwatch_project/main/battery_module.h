#ifndef BATTERY_MODULE_H
#define BATTERY_MODULE_H

// Configura el ADC y la calibración para leer la batería
void battery_init(void);

// Devuelve el voltaje real de la batería en milivoltios (mV)
int battery_get_voltage_mv(void);

// (Opcional) Devuelve un porcentaje estimado de batería (0-100%)
int battery_get_percentage(void);

#endif // BATTERY_MODULE_H