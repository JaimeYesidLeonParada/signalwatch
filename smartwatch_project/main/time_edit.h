#ifndef TIME_EDIT_H
#define TIME_EDIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Edicion de hora y fecha: logica pura, sin RTC, UI, touch ni botones.
// Quien la use (touch o botones) solo traduce su entrada a estas operaciones.
// Invariante: el estado siempre es una fecha y hora validas, asi que no hace falta
// "validar" al guardar.

#define TIME_EDIT_YEAR_MIN 2000
#define TIME_EDIT_YEAR_MAX 2099   // el PCF85063 guarda el anio en 2 digitos

// Fecha y hora desde la que se empieza a editar cuando el RTC no da una hora valida.
// Es la misma que muestra clock_format_full en ese caso.
#define TIME_EDIT_DEFAULT_YEAR   2026
#define TIME_EDIT_DEFAULT_MONTH  1
#define TIME_EDIT_DEFAULT_DAY    1
#define TIME_EDIT_DEFAULT_HOUR   0
#define TIME_EDIT_DEFAULT_MINUTE 0

typedef enum {
    TIME_FIELD_HOUR = 0,
    TIME_FIELD_MINUTE,
    TIME_FIELD_DAY,
    TIME_FIELD_MONTH,
    TIME_FIELD_YEAR,
    TIME_FIELD_COUNT
} time_field_t;

typedef struct {
    uint16_t year;     // 2000..2099
    uint8_t month;     // 1..12
    uint8_t day;       // 1..dias del mes
    uint8_t hour;      // 0..23
    uint8_t minute;    // 0..59
    time_field_t field; // campo que se esta editando
} time_edit_t;

// Carga una fecha y hora. Los valores fuera de rango se llevan al limite mas cercano
// y el dia se ajusta al mes. Queda seleccionado el campo de la hora.
void time_edit_init(time_edit_t *t, int year, int month, int day, int hour, int minute);

// Sube o baja en 1 el campo seleccionado, dando la vuelta al llegar al limite
// (59 -> 0, 12 -> 1). Si cambia el mes o el anio, el dia se ajusta al nuevo mes.
void time_edit_increment(time_edit_t *t);
void time_edit_decrement(time_edit_t *t);

// Selecciona el campo siguiente o anterior, dando la vuelta.
void time_edit_next_field(time_edit_t *t);
void time_edit_prev_field(time_edit_t *t);

// Dias del mes (28..31) teniendo en cuenta los bisiestos. month fuera de 1..12 devuelve 0.
int time_edit_days_in_month(int year, int month);

// Dia de la semana: 0=domingo..6=sabado, como lo espera el RTC.
int time_edit_weekday(int year, int month, int day);

#ifdef __cplusplus
}
#endif

#endif // TIME_EDIT_H
