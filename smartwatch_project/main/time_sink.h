#ifndef TIME_SINK_H
#define TIME_SINK_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Interfaz para "alguien que sabe guardar la hora" (hoy el RTC; manana quiza otra cosa).
// Quien tiene una hora nueva (la pantalla de configuracion, o una fuente WiFi/NTP en el
// futuro) la entrega por aqui sin saber quien la guarda. Quien la guarda no sabe de donde
// viene. Solo se conectan en app_init.c.

// Fecha y hora completas, en los mismos terminos que struct tm pero con anio completo.
typedef struct {
    uint16_t year;    // 2000..2099
    uint8_t month;    // 1..12
    uint8_t day;      // 1..31
    uint8_t dow;      // 0=domingo..6=sabado
    uint8_t hour;     // 0..23
    uint8_t minute;   // 0..59
    uint8_t second;   // 0..59
} wall_time_t;

// Guarda la hora. Devuelve true solo si quedo guardada: false si algun campo esta fuera
// de rango o si fallo el almacenamiento.
typedef bool (*time_sink_t)(const wall_time_t *t);

#ifdef __cplusplus
}
#endif

#endif // TIME_SINK_H
