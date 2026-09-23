#ifndef BATTERY_MODULE_H
#define BATTERY_MODULE_H

// Configura el ADC y la calibración para leer la batería
void battery_init(void);
int battery_get_percentage(void);

#endif // BATTERY_MODULE_H