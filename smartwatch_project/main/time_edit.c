#include "time_edit.h"

static int clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// Suma delta (+1/-1) a v dentro de lo..hi, dando la vuelta en los extremos.
static int wrap(int v, int delta, int lo, int hi)
{
    v += delta;
    if (v > hi) return lo;
    if (v < lo) return hi;
    return v;
}

int time_edit_days_in_month(int year, int month)
{
    static const uint8_t days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12) return 0;
    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return (month == 2 && leap) ? 29 : days[month - 1];
}

int time_edit_weekday(int year, int month, int day)
{
    // Algoritmo de Sakamoto.
    static const uint8_t offset[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (month < 3) year -= 1;
    return (year + year / 4 - year / 100 + year / 400 + offset[month - 1] + day) % 7;
}

// Mantiene la invariante: el dia no puede pasar de los dias del mes actual.
static void fit_day(time_edit_t *t)
{
    int max = time_edit_days_in_month(t->year, t->month);
    if (t->day > max) t->day = (uint8_t)max;
}

void time_edit_init(time_edit_t *t, int year, int month, int day, int hour, int minute)
{
    t->year = (uint16_t)clamp(year, TIME_EDIT_YEAR_MIN, TIME_EDIT_YEAR_MAX);
    t->month = (uint8_t)clamp(month, 1, 12);
    t->day = (uint8_t)clamp(day, 1, time_edit_days_in_month(t->year, t->month));
    t->hour = (uint8_t)clamp(hour, 0, 23);
    t->minute = (uint8_t)clamp(minute, 0, 59);
    t->field = TIME_FIELD_HOUR;
}

static void step(time_edit_t *t, int delta)
{
    switch (t->field) {
    case TIME_FIELD_HOUR:
        t->hour = (uint8_t)wrap(t->hour, delta, 0, 23);
        break;
    case TIME_FIELD_MINUTE:
        t->minute = (uint8_t)wrap(t->minute, delta, 0, 59);
        break;
    case TIME_FIELD_DAY:
        t->day = (uint8_t)wrap(t->day, delta, 1, time_edit_days_in_month(t->year, t->month));
        break;
    case TIME_FIELD_MONTH:
        t->month = (uint8_t)wrap(t->month, delta, 1, 12);
        fit_day(t);
        break;
    case TIME_FIELD_YEAR:
        t->year = (uint16_t)wrap(t->year, delta, TIME_EDIT_YEAR_MIN, TIME_EDIT_YEAR_MAX);
        fit_day(t);
        break;
    default:
        break;
    }
}

void time_edit_increment(time_edit_t *t) { step(t, +1); }
void time_edit_decrement(time_edit_t *t) { step(t, -1); }

void time_edit_next_field(time_edit_t *t)
{
    t->field = (time_field_t)((t->field + 1) % TIME_FIELD_COUNT);
}

void time_edit_prev_field(time_edit_t *t)
{
    t->field = (time_field_t)((t->field + TIME_FIELD_COUNT - 1) % TIME_FIELD_COUNT);
}
