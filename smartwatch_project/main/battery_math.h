#ifndef BATTERY_MATH_H
#define BATTERY_MATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BATTERY_MIN_VOLTAGE      3000   // mV: 0 %
#define BATTERY_MAX_VOLTAGE      4140   // mV: 100 %
#define CHARGING_PCT_THRESHOLD     98   // umbral empirico: con USB conectado llega cerca del 100%

// Porcentaje (0-100) de una bateria con ese voltaje en mV, lineal entre el minimo y el maximo.
int battery_percent_from_mv(int mv);

// Suavizado del porcentaje: la primera lectura lo inicializa y las siguientes pesan 0.9 el
// valor anterior y 0.1 la nueva. Se guarda con decimales; solo el resultado se redondea.
typedef struct {
    float value;
    bool has_value;
} battery_filter_t;

#define BATTERY_FILTER_INIT { 0.0f, false }

// Incorpora una lectura y devuelve el porcentaje suavizado, redondeado al entero mas cercano.
int battery_filter_update(battery_filter_t *filter, int pct);

// Con USB conectado el porcentaje llega cerca del 100 %, asi que se usa como senal de carga.
bool battery_is_charging(int pct);

#ifdef __cplusplus
}
#endif

#endif // BATTERY_MATH_H
