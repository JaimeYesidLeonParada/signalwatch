#include "clock_format.h"
#include <stdio.h>

static const char *const DAY_NAMES[]   = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
static const char *const MONTH_NAMES[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                                          "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};

static const wall_time_t DEFAULT_TIME = { .year = 2026, .month = 1, .day = 1, .dow = 4,
                                          .hour = 0, .minute = 0, .second = 0 };

void clock_format_full(const wall_time_t *t, bool valid,
                       char *time_buf, size_t time_len, char *ampm_buf, size_t ampm_len,
                       char *date_buf, size_t date_len)
{
    bool in_range = valid && t != NULL &&
                    t->year >= 2000 && t->year <= 2099 && t->month >= 1 && t->month <= 12 &&
                    t->day >= 1 && t->day <= 31 && t->dow <= 6 &&
                    t->hour <= 23 && t->minute <= 59;
    const wall_time_t *use = in_range ? t : &DEFAULT_TIME;

    char short_date[CLOCK_DATE_LEN];
    clock_format(use->hour, use->minute, use->dow, use->day, use->month, true,
                 time_buf, time_len, short_date, sizeof(short_date));
    snprintf(ampm_buf, ampm_len, "%s", use->hour < 12 ? "AM" : "PM");
    snprintf(date_buf, date_len, "%s %d", short_date, use->year);
}

void clock_format(int hr, int min, int dow, int day, int month, bool valid,
                  char *time_buf, size_t time_len, char *date_buf, size_t date_len)
{
    // Un campo fuera de rango se trata como un fallo de lectura: ademas de dar una hora
    // absurda, dow y month indexan las tablas de nombres.
    if (hr < 0 || hr > 23 || min < 0 || min > 59 || dow < 0 || dow > 6 ||
        day < 1 || day > 31 || month < 1 || month > 12) {
        valid = false;
    }

    if (!valid) hr = min = 0;   // sin lectura del RTC: se muestra 12:00 y sin fecha

    int hr12 = hr % 12;
    if (hr12 == 0) hr12 = 12;

    snprintf(time_buf, time_len, "%d:%02d", hr12, min);
    if (valid) snprintf(date_buf, date_len, "%s %d %s", DAY_NAMES[dow], day, MONTH_NAMES[month - 1]);
    else       date_buf[0] = '\0';
}
