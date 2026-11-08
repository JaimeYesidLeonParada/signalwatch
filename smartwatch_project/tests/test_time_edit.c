#include "unity.h"
#include "time_edit.h"

void setUp(void) {}
void tearDown(void) {}

// 2026-11-08 (domingo) 14:30, con el campo indicado seleccionado.
static time_edit_t make(time_field_t field)
{
    time_edit_t t;
    time_edit_init(&t, 2026, 11, 8, 14, 30);
    t.field = field;
    return t;
}

static time_edit_t make_date(int year, int month, int day, time_field_t field)
{
    time_edit_t t;
    time_edit_init(&t, year, month, day, 0, 0);
    t.field = field;
    return t;
}

// ---- time_edit_days_in_month ----

static void test_days_in_month_of_31_and_30(void)
{
    TEST_ASSERT_EQUAL_INT(31, time_edit_days_in_month(2026, 1));
    TEST_ASSERT_EQUAL_INT(30, time_edit_days_in_month(2026, 4));
    TEST_ASSERT_EQUAL_INT(31, time_edit_days_in_month(2026, 12));
}

static void test_february_has_28_days_in_a_normal_year(void)
{
    TEST_ASSERT_EQUAL_INT(28, time_edit_days_in_month(2026, 2));
}

static void test_february_has_29_days_in_a_leap_year(void)
{
    TEST_ASSERT_EQUAL_INT(29, time_edit_days_in_month(2024, 2));
}

static void test_year_2000_is_leap_by_the_400_rule(void)
{
    TEST_ASSERT_EQUAL_INT(29, time_edit_days_in_month(2000, 2));
}

static void test_century_year_is_not_leap(void)
{
    TEST_ASSERT_EQUAL_INT(28, time_edit_days_in_month(2100, 2));
}

static void test_days_in_month_rejects_invalid_month(void)
{
    TEST_ASSERT_EQUAL_INT(0, time_edit_days_in_month(2026, 0));
    TEST_ASSERT_EQUAL_INT(0, time_edit_days_in_month(2026, 13));
}

// ---- time_edit_weekday ----

static void test_weekday_of_known_dates(void)
{
    TEST_ASSERT_EQUAL_INT(0, time_edit_weekday(2026, 11, 8));   // domingo
    TEST_ASSERT_EQUAL_INT(6, time_edit_weekday(2000, 1, 1));    // sabado
    TEST_ASSERT_EQUAL_INT(4, time_edit_weekday(2024, 2, 29));   // jueves
}

// ---- time_edit_init ----

static void test_init_stores_the_values_and_selects_the_hour(void)
{
    time_edit_t t;
    time_edit_init(&t, 2026, 11, 8, 14, 30);
    TEST_ASSERT_EQUAL_UINT16(2026, t.year);
    TEST_ASSERT_EQUAL_UINT8(11, t.month);
    TEST_ASSERT_EQUAL_UINT8(8, t.day);
    TEST_ASSERT_EQUAL_UINT8(14, t.hour);
    TEST_ASSERT_EQUAL_UINT8(30, t.minute);
    TEST_ASSERT_EQUAL_INT(TIME_FIELD_HOUR, t.field);
}

static void test_init_clamps_values_below_the_range(void)
{
    time_edit_t t;
    time_edit_init(&t, 1990, 0, 0, -5, -1);
    TEST_ASSERT_EQUAL_UINT16(2000, t.year);
    TEST_ASSERT_EQUAL_UINT8(1, t.month);
    TEST_ASSERT_EQUAL_UINT8(1, t.day);
    TEST_ASSERT_EQUAL_UINT8(0, t.hour);
    TEST_ASSERT_EQUAL_UINT8(0, t.minute);
}

static void test_init_clamps_values_above_the_range(void)
{
    time_edit_t t;
    time_edit_init(&t, 2150, 13, 40, 24, 60);
    TEST_ASSERT_EQUAL_UINT16(2099, t.year);
    TEST_ASSERT_EQUAL_UINT8(12, t.month);
    TEST_ASSERT_EQUAL_UINT8(31, t.day);
    TEST_ASSERT_EQUAL_UINT8(23, t.hour);
    TEST_ASSERT_EQUAL_UINT8(59, t.minute);
}

static void test_init_fits_the_day_to_the_month(void)
{
    time_edit_t t;
    time_edit_init(&t, 2026, 2, 31, 0, 0);
    TEST_ASSERT_EQUAL_UINT8(28, t.day);
}

// ---- increment / decrement: hora y minuto ----

static void test_increment_hour(void)
{
    time_edit_t t = make(TIME_FIELD_HOUR);
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(15, t.hour);
}

static void test_hour_wraps_from_23_to_0(void)
{
    time_edit_t t = make(TIME_FIELD_HOUR);
    t.hour = 23;
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(0, t.hour);
}

static void test_hour_wraps_from_0_to_23(void)
{
    time_edit_t t = make(TIME_FIELD_HOUR);
    t.hour = 0;
    time_edit_decrement(&t);
    TEST_ASSERT_EQUAL_UINT8(23, t.hour);
}

static void test_minute_wraps_in_both_directions(void)
{
    time_edit_t t = make(TIME_FIELD_MINUTE);
    t.minute = 59;
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(0, t.minute);
    time_edit_decrement(&t);
    TEST_ASSERT_EQUAL_UINT8(59, t.minute);
}

static void test_changing_minute_does_not_touch_the_hour(void)
{
    time_edit_t t = make(TIME_FIELD_MINUTE);
    t.minute = 59;
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(14, t.hour);
}

// ---- increment / decrement: fecha ----

static void test_day_wraps_at_the_end_of_a_30_day_month(void)
{
    time_edit_t t = make_date(2026, 4, 30, TIME_FIELD_DAY);
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(1, t.day);
}

static void test_day_wraps_to_the_last_day_of_the_month(void)
{
    time_edit_t t = make_date(2024, 2, 1, TIME_FIELD_DAY);
    time_edit_decrement(&t);
    TEST_ASSERT_EQUAL_UINT8(29, t.day);
}

static void test_month_wraps_from_12_to_1(void)
{
    time_edit_t t = make_date(2026, 12, 10, TIME_FIELD_MONTH);
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT8(1, t.month);
}

static void test_changing_month_fits_the_day(void)
{
    time_edit_t t = make_date(2026, 1, 31, TIME_FIELD_MONTH);
    time_edit_increment(&t);   // enero -> febrero
    TEST_ASSERT_EQUAL_UINT8(2, t.month);
    TEST_ASSERT_EQUAL_UINT8(28, t.day);
}

static void test_changing_year_fits_feb_29(void)
{
    time_edit_t t = make_date(2024, 2, 29, TIME_FIELD_YEAR);
    time_edit_increment(&t);   // 2024 bisiesto -> 2025 no
    TEST_ASSERT_EQUAL_UINT16(2025, t.year);
    TEST_ASSERT_EQUAL_UINT8(28, t.day);
}

static void test_year_wraps_inside_the_rtc_range(void)
{
    time_edit_t t = make_date(2099, 5, 5, TIME_FIELD_YEAR);
    time_edit_increment(&t);
    TEST_ASSERT_EQUAL_UINT16(2000, t.year);
    time_edit_decrement(&t);
    TEST_ASSERT_EQUAL_UINT16(2099, t.year);
}

// ---- seleccion de campo ----

static void test_next_field_walks_all_fields_and_wraps(void)
{
    time_edit_t t = make(TIME_FIELD_HOUR);
    const time_field_t order[] = { TIME_FIELD_MINUTE, TIME_FIELD_DAY, TIME_FIELD_MONTH,
                                   TIME_FIELD_YEAR, TIME_FIELD_HOUR };
    for (int i = 0; i < 5; i++) {
        time_edit_next_field(&t);
        TEST_ASSERT_EQUAL_INT(order[i], t.field);
    }
}

static void test_prev_field_wraps_to_the_last_field(void)
{
    time_edit_t t = make(TIME_FIELD_HOUR);
    time_edit_prev_field(&t);
    TEST_ASSERT_EQUAL_INT(TIME_FIELD_YEAR, t.field);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_days_in_month_of_31_and_30);
    RUN_TEST(test_february_has_28_days_in_a_normal_year);
    RUN_TEST(test_february_has_29_days_in_a_leap_year);
    RUN_TEST(test_year_2000_is_leap_by_the_400_rule);
    RUN_TEST(test_century_year_is_not_leap);
    RUN_TEST(test_days_in_month_rejects_invalid_month);
    RUN_TEST(test_weekday_of_known_dates);
    RUN_TEST(test_init_stores_the_values_and_selects_the_hour);
    RUN_TEST(test_init_clamps_values_below_the_range);
    RUN_TEST(test_init_clamps_values_above_the_range);
    RUN_TEST(test_init_fits_the_day_to_the_month);
    RUN_TEST(test_increment_hour);
    RUN_TEST(test_hour_wraps_from_23_to_0);
    RUN_TEST(test_hour_wraps_from_0_to_23);
    RUN_TEST(test_minute_wraps_in_both_directions);
    RUN_TEST(test_changing_minute_does_not_touch_the_hour);
    RUN_TEST(test_day_wraps_at_the_end_of_a_30_day_month);
    RUN_TEST(test_day_wraps_to_the_last_day_of_the_month);
    RUN_TEST(test_month_wraps_from_12_to_1);
    RUN_TEST(test_changing_month_fits_the_day);
    RUN_TEST(test_changing_year_fits_feb_29);
    RUN_TEST(test_year_wraps_inside_the_rtc_range);
    RUN_TEST(test_next_field_walks_all_fields_and_wraps);
    RUN_TEST(test_prev_field_wraps_to_the_last_field);
    return UNITY_END();
}
