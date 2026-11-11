#include "unity.h"
#include "alert_module.h"
#include <stdio.h>

void setUp(void)
{
    alert_module_init();
}

// Los tests de ventana, umbrales y clases no dependen del tiempo: todas las muestras llegan en
// el mismo instante (0), asi que ninguna caduca.
static alert_level_t add_at_zero(const char *label, float confidence)
{
    return alert_module_add_sample(label, confidence, 0);
}

#define SEC 1000u

void tearDown(void) {}

// Con una sola muestra el promedio es la propia muestra, asi que cada borde de
// umbral se prueba con una clase distinta (una ventana limpia por valor).
static void test_level_thresholds(void)
{
    TEST_ASSERT_EQUAL(ALERT_NONE,   add_at_zero("a", 0.49f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    add_at_zero("b", 0.50f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    add_at_zero("c", 0.59f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, add_at_zero("d", 0.60f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, add_at_zero("e", 0.69f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   add_at_zero("f", 0.70f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   add_at_zero("g", 1.00f));
}

// Con menos de 3 muestras se promedia solo lo recibido, no se rellena con ceros.
static void test_partial_window_averages_only_received_samples(void)
{
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_at_zero("fire", 0.9f));
}

// 1.0, 0.0, 0.0 -> promedio 0.33 -> NONE (un pico aislado no dispara la alerta).
static void test_average_covers_the_whole_window(void)
{
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("fire", 0.0f));
}

// La 4a muestra desplaza a la 1a: 1.0, 1.0, 0.0, 0.0 -> ventana {1.0, 0.0, 0.0} = 0.33 (NONE).
// Si no se descartara ninguna, el promedio de las 4 seria 0.5 (LOW).
static void test_window_drops_the_oldest_sample(void)
{
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("fire", 0.0f));
}

static void test_labels_have_independent_windows(void)
{
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 1.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("ambulance", 0.1f));
}

static void test_init_clears_the_history(void)
{
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 1.0f);
    alert_module_init();
    // Sin historial el promedio es 0.0; con el historial anterior seria 0.67 (LOW).
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("fire", 0.0f));
}

// Al llegar al maximo (8 clases) la siguiente devuelve NONE y las ya registradas siguen igual.
static void test_ninth_label_is_ignored_and_others_keep_working(void)
{
    char label[8];
    for (int i = 0; i < 8; i++) {
        snprintf(label, sizeof(label), "c%d", i);
        add_at_zero(label, 0.9f);
    }
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("c8", 1.0f));
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_at_zero("c0", 0.9f));
}

// Una etiqueta de mas de 31 caracteres se guarda recortada: tiene que seguir
// encontrando su ventana en vez de ocupar una ranura nueva en cada muestra.
static void test_long_label_keeps_a_single_window(void)
{
    const char *long_label = "0123456789012345678901234567890123456789";
    for (int i = 0; i < 20; i++) {
        add_at_zero(long_label, 0.1f);
    }
    // Si hubiera agotado las 8 ranuras, esta clase nueva devolveria NONE.
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_at_zero("fire", 0.9f));
}

// Dos lecturas altas, un largo silencio y una lectura debil: sin caducidad el promedio seria
// (0.95 + 0.9 + 0.3) / 3 = 0.72 (HIGH); las viejas ya no cuentan, asi que solo queda 0.3.
static void test_old_samples_do_not_count_after_a_long_silence(void)
{
    alert_module_add_sample("baby-crying", 0.95f, 0);
    alert_module_add_sample("baby-crying", 0.90f, 2 * SEC);
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("baby-crying", 0.3f, 600 * SEC));
}

// Las muestras de hace menos del limite si cuentan.
static void test_recent_samples_still_count(void)
{
    alert_module_add_sample("baby-crying", 0.95f, 0);
    alert_module_add_sample("baby-crying", 0.90f, 2 * SEC);
    TEST_ASSERT_EQUAL(ALERT_HIGH, alert_module_add_sample("baby-crying", 0.30f, 4 * SEC));
}

// El borde: justo en el limite todavia cuenta; un milisegundo mas y ya no.
static void test_a_sample_expires_just_after_the_maximum_age(void)
{
    alert_module_add_sample("fire", 1.0f, 0);
    TEST_ASSERT_EQUAL(ALERT_LOW, alert_module_add_sample("fire", 0.0f, ALERT_SAMPLE_MAX_AGE_MS));
    alert_module_init();
    alert_module_add_sample("fire", 1.0f, 0);
    TEST_ASSERT_EQUAL(ALERT_NONE, alert_module_add_sample("fire", 0.0f, ALERT_SAMPLE_MAX_AGE_MS + 1));
}

// El contador de milisegundos de 32 bits da la vuelta cada ~49 dias; no debe borrar muestras
// recientes ni conservar las viejas.
static void test_counter_wrap_around_keeps_recent_samples(void)
{
    uint32_t before_wrap = 0xFFFFFFFFu - 1000u;
    alert_module_add_sample("fire", 0.9f, before_wrap);
    TEST_ASSERT_EQUAL(ALERT_HIGH, alert_module_add_sample("fire", 0.8f, before_wrap + 2000u));
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
    RUN_TEST(test_old_samples_do_not_count_after_a_long_silence);
    RUN_TEST(test_recent_samples_still_count);
    RUN_TEST(test_a_sample_expires_just_after_the_maximum_age);
    RUN_TEST(test_counter_wrap_around_keeps_recent_samples);
    return UNITY_END();
}
