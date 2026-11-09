#ifndef TIME_SOURCE_H
#define TIME_SOURCE_H

#include <stdbool.h>
#include "time_sink.h"   // wall_time_t

#ifdef __cplusplus
extern "C" {
#endif

// Interfaz para "alguien que sabe decir la hora actual" (hoy el RTC). Es la pareja de
// time_sink_t, separada a proposito: quien solo muestra la hora no necesita poder cambiarla.
// Devuelve true si out se lleno con una hora valida; false si no hay hora (el chip no
// responde, o la hora guardada no es coherente).
typedef bool (*time_source_t)(wall_time_t *out);

#ifdef __cplusplus
}
#endif

#endif // TIME_SOURCE_H
