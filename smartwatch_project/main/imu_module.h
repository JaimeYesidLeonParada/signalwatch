#ifndef IMU_MODULE_H
#define IMU_MODULE_H

#include <stdbool.h>
#include "driver/i2c_master.h"

// Acelerometro QMI8658 de la placa. Solo lee la aceleracion y la entrega por callback; no sabe
// que se hace con ella (el gesto de muneca esta en wrist_gesture).
//
// Las muestras salen ya en la convencion de wrist_gesture.h: milli-g, az positivo cuando la
// pantalla mira hacia arriba. Como esta montado el sensor respecto a la pantalla se ajusta con
// IMU_AXIS_* en imu_module.c.

typedef void (*imu_sample_cb_t)(int ax_mg, int ay_mg, int az_mg);

// Busca el sensor en el bus I2C y lo configura. Devuelve false si no responde (el resto de la
// API entonces no hace nada).
bool imu_init(i2c_master_bus_handle_t bus_handle);

// Lanza la tarea que entrega una muestra cada WRIST_GESTURE_SAMPLE_PERIOD_MS mientras esta
// activado. Empieza desactivado.
void imu_start(imu_sample_cb_t on_sample);

// Activa o desactiva la entrega de muestras. Desactivado, la tarea duerme y no usa el bus I2C.
// Se puede llamar desde cualquier tarea.
void imu_set_enabled(bool enabled);

#endif // IMU_MODULE_H
