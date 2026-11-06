#include "unity.h"
#include "battery_math.h"

void setUp(void) {}
void tearDown(void) {}

// --- battery_percent_from_mv -----------------------------------------------

static void test_percent_at_the_ends_of_the_range(void)
{
    TEST_ASSERT_EQUAL(0,   battery_percent_from_mv(BATTERY_MIN_VOLTAGE));
    TEST_ASSERT_EQUAL(100, battery_percent_from_mv(BATTERY_MAX_VOLTAGE));
}

// Fuera del rango (bateria agotada o con USB conectado) se recorta a 0 y 100.
static void test_percent_is_clamped_outside_the_range(void)
{
    TEST_ASSERT_EQUAL(0,   battery_percent_from_mv(0));
    TEST_ASSERT_EQUAL(0,   battery_percent_from_mv(BATTERY_MIN_VOLTAGE - 1));
    TEST_ASSERT_EQUAL(100, battery_percent_from_mv(BATTERY_MAX_VOLTAGE + 1));
    TEST_ASSERT_EQUAL(100, battery_percent_from_mv(5000));
}

// Lineal: 3570 mV esta justo en la mitad entre 3000 y 4140.
static void test_percent_is_linear_between_the_limits(void)
{
    TEST_ASSERT_EQUAL(50, battery_percent_from_mv(3570));
    TEST_ASSERT_EQUAL(0,  battery_percent_from_mv(BATTERY_MIN_VOLTAGE + 1));
    TEST_ASSERT_EQUAL(99, battery_percent_from_mv(BATTERY_MAX_VOLTAGE - 1));
}

// Mas voltaje nunca da menos porcentaje, en todo el rango de la bateria.
static void test_percent_never_decreases_as_voltage_rises(void)
{
    int previous = battery_percent_from_mv(2500);
    for (int mv = 2501; mv <= 4500; mv++) {
        int pct = battery_percent_from_mv(mv);
        TEST_ASSERT_TRUE(pct >= previous);
        TEST_ASSERT_TRUE(pct >= 0 && pct <= 100);
        previous = pct;
    }
}

// --- battery_filter_update -------------------------------------------------

static void test_first_reading_is_taken_as_is(void)
{
    battery_filter_t f = BATTERY_FILTER_INIT;
    TEST_ASSERT_EQUAL(80, battery_filter_update(&f, 80));
}

// Una primera lectura de 0 % tambien cuenta como lectura: la siguiente ya se suaviza
// (0.9 * 0 + 0.1 * 100 = 10) en vez de volver a tomarse tal cual.
static void test_first_reading_of_zero_still_initializes_the_filter(void)
{
    battery_filter_t f = BATTERY_FILTER_INIT;
    TEST_ASSERT_EQUAL(0,  battery_filter_update(&f, 0));
    TEST_ASSERT_EQUAL(10, battery_filter_update(&f, 100));
}

// 100 -> 0: primero 90 (0.9 * 100), luego 81 (0.9 * 90).
static void test_new_readings_weigh_ten_percent(void)
{
    battery_filter_t f = BATTERY_FILTER_INIT;
    battery_filter_update(&f, 100);
    TEST_ASSERT_EQUAL(90, battery_filter_update(&f, 0));
    TEST_ASSERT_EQUAL(81, battery_filter_update(&f, 0));
}

// La salida es el entero mas cercano: 0.6 sube a 1 y 0.4 baja a 0 (truncar daria 0 en los dos).
static void test_output_is_rounded_to_the_nearest_integer(void)
{
    battery_filter_t up = BATTERY_FILTER_INIT, down = BATTERY_FILTER_INIT;
    battery_filter_update(&up, 0);
    battery_filter_update(&down, 0);
    TEST_ASSERT_EQUAL(1, battery_filter_update(&up, 6));
    TEST_ASSERT_EQUAL(0, battery_filter_update(&down, 4));
}

// El filtro guarda el valor con decimales; si guardara el entero redondeado, una lectura
// 4 puntos por encima nunca lograria subir la salida (0.9 * 0 + 0.4 redondea siempre a 0).
static void test_filter_keeps_the_fraction_between_readings(void)
{
    battery_filter_t f = BATTERY_FILTER_INIT;
    battery_filter_update(&f, 0);
    int out = 0;
    for (int i = 0; i < 3; i++) out = battery_filter_update(&f, 4);
    TEST_ASSERT_TRUE(out >= 1);
}

// Con lecturas constantes la salida no se mueve ni deriva.
static void test_constant_readings_keep_the_output_steady(void)
{
    battery_filter_t f = BATTERY_FILTER_INIT;
    battery_filter_update(&f, 40);
    for (int i = 0; i < 200; i++) {
        TEST_ASSERT_EQUAL(40, battery_filter_update(&f, 40));
    }
}

// Cada filtro lleva su propio estado.
static void test_filters_are_independent(void)
{
    battery_filter_t a = BATTERY_FILTER_INIT, b = BATTERY_FILTER_INIT;
    battery_filter_update(&a, 100);
    TEST_ASSERT_EQUAL(20, battery_filter_update(&b, 20));
}

// --- battery_is_charging ---------------------------------------------------

static void test_charging_threshold(void)
{
    TEST_ASSERT_FALSE(battery_is_charging(CHARGING_PCT_THRESHOLD - 1));
    TEST_ASSERT_TRUE(battery_is_charging(CHARGING_PCT_THRESHOLD));
    TEST_ASSERT_TRUE(battery_is_charging(100));
    TEST_ASSERT_FALSE(battery_is_charging(0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_percent_at_the_ends_of_the_range);
    RUN_TEST(test_percent_is_clamped_outside_the_range);
    RUN_TEST(test_percent_is_linear_between_the_limits);
    RUN_TEST(test_percent_never_decreases_as_voltage_rises);
    RUN_TEST(test_first_reading_is_taken_as_is);
    RUN_TEST(test_first_reading_of_zero_still_initializes_the_filter);
    RUN_TEST(test_new_readings_weigh_ten_percent);
    RUN_TEST(test_output_is_rounded_to_the_nearest_integer);
    RUN_TEST(test_filter_keeps_the_fraction_between_readings);
    RUN_TEST(test_constant_readings_keep_the_output_steady);
    RUN_TEST(test_filters_are_independent);
    RUN_TEST(test_charging_threshold);
    return UNITY_END();
}
