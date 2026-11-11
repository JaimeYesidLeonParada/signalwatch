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

// Dos muestras iguales: con "las 2 mejores de las ultimas 4", repetir el valor hace que el
// resultado sea ese mismo valor. Devuelve el nivel tras la segunda.
static alert_level_t add_twice(const char *label, float confidence)
{
    add_at_zero(label, confidence);
    return add_at_zero(label, confidence);
}

#define SEC 1000u

void tearDown(void) {}

// Con dos muestras iguales el promedio de las 2 mejores es ese valor, asi que cada borde de
// umbral se prueba con una clase distinta (una ventana limpia por valor).
static void test_level_thresholds(void)
{
    TEST_ASSERT_EQUAL(ALERT_NONE,   add_twice("a", 0.49f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    add_twice("b", 0.50f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    add_twice("c", 0.59f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, add_twice("d", 0.60f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, add_twice("e", 0.69f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   add_twice("f", 0.70f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   add_twice("g", 1.00f));
}

// Una sola deteccion no alcanza: las muestras que faltan valen 0, asi que 0.98 da 0.49 (NONE).
// Antes bastaba una muestra; ahora hace falta que el sonido se repita.
static void test_a_single_detection_alone_does_not_alert(void)
{
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("fire", 0.98f));
}

// Dos detecciones seguidas si alertan.
static void test_two_good_detections_alert(void)
{
    add_at_zero("fire", 0.9f);
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_at_zero("fire", 0.9f));
}

// Una alarma de pitidos solo se reconoce en parte de las ventanas (las demas caen en las
// pausas). 0.98, 0, 0, 0.85 daba un promedio de 0.46 (NONE) con todas las muestras; con las 2
// mejores es 0.915 (HIGH).
static void test_pauses_between_detections_do_not_dilute_them(void)
{
    add_at_zero("fire", 0.98f);
    add_at_zero("fire", 0.0f);
    add_at_zero("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_at_zero("fire", 0.85f));
}

// Un pico aislado seguido de silencio no alerta: 0.98, 0, 0 -> (0.98 + 0) / 2 = 0.49.
static void test_an_isolated_peak_followed_by_silence_does_not_alert(void)
{
    add_at_zero("fire", 0.98f);
    add_at_zero("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_NONE, add_at_zero("fire", 0.0f));
}

// La 5a muestra desplaza a la 1a: 1.0, 1.0, 0.0, 0.0, 0.0 -> ventana {1.0, 0.0, 0.0, 0.0} ->
// las 2 mejores dan 0.5 (LOW). Si no se descartara ninguna, serian 1.0 y 1.0 (HIGH).
static void test_window_drops_the_oldest_sample(void)
{
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 1.0f);
    add_at_zero("fire", 0.0f);
    add_at_zero("fire", 0.0f);
    TEST_ASSERT_EQUAL(ALERT_LOW, add_at_zero("fire", 0.0f));
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
    // Sin historial el resultado es 0.0; con el historial anterior serian 1.0 y 1.0 (HIGH).
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
    TEST_ASSERT_EQUAL(ALERT_NONE, add_twice("c8", 1.0f));
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
    TEST_ASSERT_EQUAL(ALERT_HIGH, add_twice("fire", 0.9f));
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
    RUN_TEST(test_a_single_detection_alone_does_not_alert);
    RUN_TEST(test_two_good_detections_alert);
    RUN_TEST(test_pauses_between_detections_do_not_dilute_them);
    RUN_TEST(test_an_isolated_peak_followed_by_silence_does_not_alert);
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
