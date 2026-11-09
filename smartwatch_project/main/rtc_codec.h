#ifndef RTC_CODEC_H
#define RTC_CODEC_H

#include <stdbool.h>
#include <stdint.h>
#include "time_sink.h"

#ifdef __cplusplus
extern "C" {
#endif

// Hora tal como la guarda el chip. dow: 0=domingo..6=sabado; month: 1..12; year: 0..99.
typedef struct {
    uint8_t sec, min, hr;
    uint8_t day, dow, month, year;
} rtc_time_t;

// Bytes que se leen del PCF85063 a partir del registro 0x04 (segundos).
#define RTC_RAW_LEN 7

// Convierte los registros del chip (BCD, con bits de control) en una hora.
// Devuelve false si algun campo queda fuera de rango (hr 0-23, min 0-59, dow 0-6,
// day 1-31, month 1-12); out se llena igual.
bool rtc_decode_registers(const uint8_t raw[RTC_RAW_LEN], rtc_time_t *out);

// Convierte una hora en los registros del chip (BCD) para escribirlos a partir de 0x04.
// Devuelve false, sin tocar raw, si algun campo esta fuera de rango (year 2000-2099,
// month 1-12, day 1-31, dow 0-6, hour 0-23, minute 0-59, second 0-59). No comprueba que
// el dia exista en ese mes: eso es responsabilidad de quien arma la fecha.
bool rtc_encode_registers(const wall_time_t *t, uint8_t raw[RTC_RAW_LEN]);

#ifdef __cplusplus
}
#endif

#endif // RTC_CODEC_H
