#include "unity.h"
#include "clock_format.h"
#include <stdio.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

typedef struct {
    char time[CLOCK_TIME_LEN];
    char date[CLOCK_DATE_LEN];
} texts_t;

static texts_t format(int hr, int min, int dow, int day, int month, bool valid)
{
    texts_t t;
    clock_format(hr, min, dow, day, month, valid, t.time, sizeof(t.time), t.date, sizeof(t.date));
    return t;
}

// Hora de 12 horas para las 24 horas del dia (0 y 12 se muestran como 12).
static void test_twelve_hour_conversion(void)
{
    const int expected[24] = { 12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                               12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    for (int hr = 0; hr < 24; hr++) {
        char want[CLOCK_TIME_LEN];
        snprintf(want, sizeof(want), "%d:30", expected[hr]);
        TEST_ASSERT_EQUAL_STRING(want, format(hr, 30, 3, 17, 11, true).time);
    }
}

static void test_minutes_always_have_two_digits(void)
{
    TEST_ASSERT_EQUAL_STRING("2:00",  format(2, 0,  3, 17, 11, true).time);
    TEST_ASSERT_EQUAL_STRING("2:05",  format(2, 5,  3, 17, 11, true).time);
    TEST_ASSERT_EQUAL_STRING("2:59",  format(2, 59, 3, 17, 11, true).time);
    TEST_ASSERT_EQUAL_STRING("11:07", format(23, 7, 3, 17, 11, true).time);
}

static void test_date_format(void)
{
    TEST_ASSERT_EQUAL_STRING("Mie 17 Nov", format(14, 35, 3, 17, 11, true).date);
    TEST_ASSERT_EQUAL_STRING("Dom 5 Ene",  format(14, 35, 0, 5, 1, true).date);   // el dia no lleva cero a la izquierda
}

static void test_every_weekday_name(void)
{
    const char *expected[7] = { "Dom 1 Ene", "Lun 1 Ene", "Mar 1 Ene", "Mie 1 Ene",
                                "Jue 1 Ene", "Vie 1 Ene", "Sab 1 Ene" };
    for (int dow = 0; dow < 7; dow++) {
        TEST_ASSERT_EQUAL_STRING(expected[dow], format(12, 0, dow, 1, 1, true).date);
    }
}

static void test_every_month_name(void)
{
    const char *expected[12] = { "Dom 1 Ene", "Dom 1 Feb", "Dom 1 Mar", "Dom 1 Abr",
                                 "Dom 1 May", "Dom 1 Jun", "Dom 1 Jul", "Dom 1 Ago",
                                 "Dom 1 Sep", "Dom 1 Oct", "Dom 1 Nov", "Dom 1 Dic" };
    for (int month = 1; month <= 12; month++) {
        TEST_ASSERT_EQUAL_STRING(expected[month - 1], format(12, 0, 0, 1, month, true).date);
    }
}

// Sin lectura del RTC: 12:00 y sin fecha, sea cual sea el resto de los argumentos.
static void test_invalid_reading_shows_noon_and_no_date(void)
{
    texts_t t = format(15, 42, 3, 17, 11, false);
    TEST_ASSERT_EQUAL_STRING("12:00", t.time);
    TEST_ASSERT_EQUAL_STRING("", t.date);
}

// Un campo fuera de rango se trata como un fallo de lectura, aunque valid sea true.
static void test_out_of_range_fields_are_treated_as_invalid(void)
{
    const texts_t bad[] = {
        format(24, 30, 3, 17, 11, true), format(-1, 30, 3, 17, 11, true),
        format(14, 60, 3, 17, 11, true), format(14, -1, 3, 17, 11, true),
        format(14, 30, 7, 17, 11, true), format(14, 30, -1, 17, 11, true),
        format(14, 30, 3, 0,  11, true), format(14, 30, 3, 32, 11, true),
        format(14, 30, 3, 17, 0,  true), format(14, 30, 3, 17, 13, true),
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        TEST_ASSERT_EQUAL_STRING("12:00", bad[i].time);
        TEST_ASSERT_EQUAL_STRING("", bad[i].date);
    }
}

// Los bordes validos no se confunden con fuera de rango.
static void test_range_limits_are_accepted(void)
{
    texts_t first = format(0, 0, 0, 1, 1, true);
    texts_t last  = format(23, 59, 6, 31, 12, true);
    TEST_ASSERT_EQUAL_STRING("12:00", first.time);
    TEST_ASSERT_EQUAL_STRING("Dom 1 Ene", first.date);
    TEST_ASSERT_EQUAL_STRING("11:59", last.time);
    TEST_ASSERT_EQUAL_STRING("Sab 31 Dic", last.date);
}

// snprintf recortaria en silencio un texto que no cabe: los tamanos tienen que alcanzar
// para cualquier hora y fecha validas.
static void test_texts_fit_their_buffers(void)
{
    char big_time[64], big_date[64];
    for (int hr = 0; hr < 24; hr++) {
        for (int min = 0; min < 60; min++) {
            texts_t t = format(hr, min, 0, 1, 1, true);
            clock_format(hr, min, 0, 1, 1, true, big_time, sizeof(big_time), big_date, sizeof(big_date));
            TEST_ASSERT_EQUAL_STRING(big_time, t.time);
        }
    }
    for (int dow = 0; dow < 7; dow++) {
        for (int day = 1; day <= 31; day++) {
            for (int month = 1; month <= 12; month++) {
                texts_t t = format(12, 0, dow, day, month, true);
                clock_format(12, 0, dow, day, month, true, big_time, sizeof(big_time), big_date, sizeof(big_date));
                TEST_ASSERT_EQUAL_STRING(big_date, t.date);
            }
        }
    }
}

// ---- clock_format_full (pantalla de ajustes) ----

typedef struct {
    char time[CLOCK_TIME_LEN];
    char ampm[CLOCK_AMPM_LEN];
    char date[CLOCK_FULL_DATE_LEN];
} full_texts_t;

static full_texts_t format_full(const wall_time_t *t, bool valid)
{
    full_texts_t f;
    clock_format_full(t, valid, f.time, sizeof(f.time), f.ampm, sizeof(f.ampm), f.date, sizeof(f.date));
    return f;
}

// 2026-11-08 (domingo) con la hora indicada.
static wall_time_t at(int hour, int minute)
{
    wall_time_t t = { .year = 2026, .month = 11, .day = 8, .dow = 0,
                      .hour = (uint8_t)hour, .minute = (uint8_t)minute, .second = 0 };
    return t;
}

static void test_full_format_of_an_afternoon_time(void)
{
    wall_time_t t = at(14, 30);
    full_texts_t f = format_full(&t, true);
    TEST_ASSERT_EQUAL_STRING("2:30", f.time);
    TEST_ASSERT_EQUAL_STRING("PM", f.ampm);
    TEST_ASSERT_EQUAL_STRING("Dom 8 Nov 2026", f.date);
}

static void test_full_format_of_a_morning_time(void)
{
    wall_time_t t = at(9, 5);
    full_texts_t f = format_full(&t, true);
    TEST_ASSERT_EQUAL_STRING("9:05", f.time);
    TEST_ASSERT_EQUAL_STRING("AM", f.ampm);
}

// Medianoche es 12 AM y mediodia es 12 PM.
static void test_full_format_midnight_and_noon(void)
{
    wall_time_t midnight = at(0, 0), noon = at(12, 0);
    TEST_ASSERT_EQUAL_STRING("12:00", format_full(&midnight, true).time);
    TEST_ASSERT_EQUAL_STRING("AM",    format_full(&midnight, true).ampm);
    TEST_ASSERT_EQUAL_STRING("12:00", format_full(&noon, true).time);
    TEST_ASSERT_EQUAL_STRING("PM",    format_full(&noon, true).ampm);
}

static void test_full_format_without_a_valid_time_shows_the_default(void)
{
    wall_time_t t = at(14, 30);
    full_texts_t f = format_full(&t, false);
    TEST_ASSERT_EQUAL_STRING("12:00", f.time);
    TEST_ASSERT_EQUAL_STRING("AM", f.ampm);
    TEST_ASSERT_EQUAL_STRING("Jue 1 Ene 2026", f.date);
}

static void test_full_format_with_null_time_shows_the_default(void)
{
    TEST_ASSERT_EQUAL_STRING("Jue 1 Ene 2026", format_full(NULL, true).date);
}

static void test_full_format_out_of_range_year_shows_the_default(void)
{
    wall_time_t t = at(14, 30);
    t.year = 2100;
    TEST_ASSERT_EQUAL_STRING("Jue 1 Ene 2026", format_full(&t, true).date);
    t.year = 1999;
    TEST_ASSERT_EQUAL_STRING("Jue 1 Ene 2026", format_full(&t, true).date);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_full_format_of_an_afternoon_time);
    RUN_TEST(test_full_format_of_a_morning_time);
    RUN_TEST(test_full_format_midnight_and_noon);
    RUN_TEST(test_full_format_without_a_valid_time_shows_the_default);
    RUN_TEST(test_full_format_with_null_time_shows_the_default);
    RUN_TEST(test_full_format_out_of_range_year_shows_the_default);
    RUN_TEST(test_twelve_hour_conversion);
    RUN_TEST(test_minutes_always_have_two_digits);
    RUN_TEST(test_date_format);
    RUN_TEST(test_every_weekday_name);
    RUN_TEST(test_every_month_name);
    RUN_TEST(test_invalid_reading_shows_noon_and_no_date);
    RUN_TEST(test_out_of_range_fields_are_treated_as_invalid);
    RUN_TEST(test_range_limits_are_accepted);
    RUN_TEST(test_texts_fit_their_buffers);
    return UNITY_END();
}
