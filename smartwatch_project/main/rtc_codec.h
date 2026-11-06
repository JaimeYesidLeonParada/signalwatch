#ifndef RTC_CODEC_H
#define RTC_CODEC_H

#include <stdbool.h>
#include <stdint.h>

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

#ifdef __cplusplus
}
#endif

#endif // RTC_CODEC_H
