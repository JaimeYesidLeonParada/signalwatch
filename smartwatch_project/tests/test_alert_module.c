#include "unity.h"
#include "alert_module.h"
#include <stdio.h>

void setUp(void)
{
    alert_module_init();
}

void tearDown(void) {}

// Con una sola muestra el promedio es la propia muestra, asi que cada borde de
// umbral se prueba con una clase distinta (una ventana limpia por valor).
static void test_level_thresholds(void)
{
    TEST_ASSERT_EQUAL(ALERT_NONE,   alert_module_add_sample("a", 0.49f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    alert_module_add_sample("b", 0.50f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    alert_module_add_sample("c", 0.59f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, alert_module_add_sample("d", 0.60f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, alert_module_add_sample("e", 0.69f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   alert_module_add_sample("f", 0.70f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   alert_module_add_sample("g", 1.00f));
}

// Con menos de 3 muestras se promedia solo lo recibido, no se rellena con ceros.
static void test_partial_window_averages_only_received_samples(void)
{
    TEST_ASSERT_EQUAL(ALERT_HIGH, alert_module_add_sample("fire", 0.9f));
}

// 1.0, 0.0, 0.0 -> promedio 0.33 -> NONE (un pico aislado no dispara la alerta).
static void test_average_covers_the_whole_window(void)
{
    alert_module_add_sample("fire", 1.0f);
    alert_module_add_sample("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("fire", 0.0f));
}

// La 4a muestra desplaza a la 1a: 1.0, 1.0, 0.0, 0.0 -> ventana {1.0, 0.0, 0.0} = 0.33 (NONE).
// Si no se descartara ninguna, el promedio de las 4 seria 0.5 (LOW).
static void test_window_drops_the_oldest_sample(void)
{
    alert_module_add_sample("fire", 1.0f);
    alert_module_add_sample("fire", 1.0f);
    alert_module_add_sample("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("fire", 0.0f));
}

static void test_labels_have_independent_windows(void)
{
    alert_module_add_sample("fire", 1.0f);
    alert_module_add_sample("fire", 1.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("ambulance", 0.1f));
}

static void test_init_clears_the_history(void)
{
    alert_module_add_sample("fire", 1.0f);
    alert_module_add_sample("fire", 1.0f);
    alert_module_init();
    // Sin historial el promedio es 0.0; con el historial anterior seria 0.67 (LOW).
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("fire", 0.0f));
}

// Al llegar al maximo (8 clases) la siguiente devuelve NONE y las ya registradas siguen igual.
static void test_ninth_label_is_ignored_and_others_keep_working(void)
{
    char label[8];
    for (int i = 0; i < 8; i++) {
        snprintf(label, sizeof(label), "c%d", i);
        alert_module_add_sample(label, 0.9f);
    }
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("c8", 1.0f));
    TEST_ASSERT_EQUAL(ALERT_HIGH, alert_module_add_sample("c0", 0.9f));
}

// Una etiqueta de mas de 31 caracteres se guarda recortada: tiene que seguir
// encontrando su ventana en vez de ocupar una ranura nueva en cada muestra.
static void test_long_label_keeps_a_single_window(void)
{
    const char *long_label = "0123456789012345678901234567890123456789";
    for (int i = 0; i < 20; i++) {
        alert_module_add_sample(long_label, 0.1f);
    }
    // Si hubiera agotado las 8 ranuras, esta clase nueva devolveria NONE.
    TEST_ASSERT_EQUAL(ALERT_HIGH, alert_module_add_sample("fire", 0.9f));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_level_thresholds);
    RUN_TEST(test_partial_window_averages_only_received_samples);
    RUN_TEST(test_average_covers_the_whole_window);
    RUN_TEST(test_window_drops_the_oldest_sample);
    RUN_TEST(test_labels_have_independent_windows);
    RUN_TEST(test_init_clears_the_history);
    RUN_TEST(test_ninth_label_is_ignored_and_others_keep_working);
    RUN_TEST(test_long_label_keeps_a_single_window);
    return UNITY_END();
}
