#ifndef CLOCK_FORMAT_H
#define CLOCK_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

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

#ifdef __cplusplus
}
#endif

#endif // CLOCK_FORMAT_H
