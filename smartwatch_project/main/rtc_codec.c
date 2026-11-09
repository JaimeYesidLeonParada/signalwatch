#include "rtc_codec.h"

static uint8_t bcd2dec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

static uint8_t dec2bcd(uint8_t val) { return (uint8_t)((val / 10 * 16) + (val % 10)); }

bool rtc_encode_registers(const wall_time_t *t, uint8_t raw[RTC_RAW_LEN])
{
    bool in_range = t->year >= 2000 && t->year <= 2099 &&
                    t->month >= 1 && t->month <= 12 &&
                    t->day >= 1 && t->day <= 31 && t->dow <= 6 &&
                    t->hour <= 23 && t->minute <= 59 && t->second <= 59;
    if (!in_range) return false;

    // Segundos sin bit 7: escribirlo en 0 limpia la bandera "oscilador parado" del chip.
    raw[0] = dec2bcd(t->second);
    raw[1] = dec2bcd(t->minute);
    raw[2] = dec2bcd(t->hour);
    raw[3] = dec2bcd(t->day);
    raw[4] = dec2bcd(t->dow);
    raw[5] = dec2bcd(t->month);
    raw[6] = dec2bcd((uint8_t)(t->year - 2000));
    return true;
}

bool rtc_decode_registers(const uint8_t raw[RTC_RAW_LEN], rtc_time_t *out)
{
    out->sec   = bcd2dec(raw[0] & 0x7F);
    out->min   = bcd2dec(raw[1] & 0x7F);
    out->hr    = bcd2dec(raw[2] & 0x3F);
    out->day   = bcd2dec(raw[3] & 0x3F);
    out->dow   = bcd2dec(raw[4] & 0x3F);
    out->month = bcd2dec(raw[5] & 0x1F);
    out->year  = bcd2dec(raw[6]);

    // Con el chip sin configurar o sin pila los campos pueden salir fuera de rango;
    // se descartan para que nadie indexe tablas con ellos.
    bool plausible = out->hr <= 23 && out->min <= 59 && out->dow <= 6 &&
                     out->day >= 1 && out->day <= 31 && out->month >= 1 && out->month <= 12;
    return plausible;
}
