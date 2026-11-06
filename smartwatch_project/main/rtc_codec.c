#include "rtc_codec.h"

static uint8_t bcd2dec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

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
