#ifndef CLOCK_FORMAT_H
#define CLOCK_FORMAT_H

#include <stdbool.h>
#include <stddef.h>
#include "time_sink.h"   // wall_time_t

#ifdef __cplusplus
extern "C" {
#endif

// Tamanos que alcanzan para los textos ("12:59", "Mie 17 Nov") con su terminador.
#define CLOCK_TIME_LEN 8
#define CLOCK_DATE_LEN 16

// Arma los textos del reloj: la hora en formato de 12 horas ("2:05") y la fecha ("Mie 17 Nov").
// dow: 0=domingo..6=sabado; month: 1..12. Si valid es false, o si algun campo esta fuera de
// rango (hr 0-23, min 0-59, dow 0-6, day 1-31, month 1-12), la hora es "12:00" y la fecha queda vacia.
void clock_format(int hr, int min, int dow, int day, int month, bool valid,
                  char *time_buf, size_t time_len, char *date_buf, size_t date_len);

// Textos para la pantalla de ajustes, cada uno en su propio campo porque cada parte de la
// fecha es una zona distinta.
#define CLOCK_AMPM_LEN    3    // "AM"
#define CLOCK_DAY_LEN     4    // "31" (el tipo admite hasta 3 digitos)
#define CLOCK_MONTH_LEN   4    // "Nov"
#define CLOCK_YEAR_LEN    6    // "2026" (el tipo admite hasta 5 digitos)
typedef struct {
    char time[CLOCK_TIME_LEN];        // hora de 12 horas sin AM/PM ("2:05")
    char ampm[CLOCK_AMPM_LEN];
    char day[CLOCK_DAY_LEN];          // sin cero a la izquierda ("8")
    char month[CLOCK_MONTH_LEN];      // abreviado ("Nov")
    char year[CLOCK_YEAR_LEN];
} clock_texts_t;

// Si valid es false, t es NULL o algun campo esta fuera de rango (year 2000-2099, month 1-12,
// day 1-31, dow 0-6, hour 0-23, minute 0-59), se llena con la hora por defecto: 12:00 AM del
// Jue 1 Ene 2026 (desde donde el usuario empezaria a editar).
void clock_format_full(const wall_time_t *t, bool valid, clock_texts_t *out);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_FORMAT_H
