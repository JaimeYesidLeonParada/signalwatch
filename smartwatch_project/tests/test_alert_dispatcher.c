#include "unity.h"
#include "alert_dispatcher.h"
#include "alert_module.h"
#include "alert_catalog.h"
#include <string.h>

// --- Salidas falsas: guardan lo que el dispatcher les pide -----------------

#define MAX_CALLS 16

static int   audio_levels[MAX_CALLS];
static int   audio_level_count;

static char  notifications[MAX_CALLS][64];
static int   notification_count;

static char  vibrations[MAX_CALLS][64];
static int   vibration_count;

static struct { char label[32]; float confidence; } inferences[MAX_CALLS];
static int   inference_count;

static struct { char label[32]; int level; } icons[MAX_CALLS];
static int   icon_count;

static void fake_audio_level(int level_pct)
{
    audio_levels[audio_level_count++] = level_pct;
}

static void fake_notification(const char *text)
{
    strcpy(notifications[notification_count++], text);
}

static void fake_vibrate(const char *text)
{
    strcpy(vibrations[vibration_count++], text);
}

static void fake_inference(const char *label, float confidence)
{
    strcpy(inferences[inference_count].label, label);
    inferences[inference_count].confidence = confidence;
    inference_count++;
}

static void fake_alert_icon(const char *label, int level)
{
    strcpy(icons[icon_count].label, label);
    icons[icon_count].level = level;
    icon_count++;
}

static const alert_outputs_t OUTPUTS = {
    .audio_level   = fake_audio_level,
    .inference     = fake_inference,
    .alert_icon    = fake_alert_icon,
    .notification  = fake_notification,
    .vibrate_morse = fake_vibrate,
};

void setUp(void)
{
    audio_level_count = notification_count = vibration_count = inference_count = icon_count = 0;
    alert_dispatcher_init(&OUTPUTS);
}

void tearDown(void) {}

// --- Ayudas ----------------------------------------------------------------

static const char *catalog_label(size_t index) { return alert_catalog_get(index)->label; }
static const char *catalog_morse(size_t index) { return alert_catalog_get(index)->morse; }

// Resultado de una clasificacion con una sola clase.
static ei_wrapper_result_t single(const char *label, float confidence)
{
    ei_wrapper_result_t r;
    memset(&r, 0, sizeof(r));
    strcpy(r.classifications[0].label, label);
    r.classifications[0].confidence = confidence;
    r.count = 1;
    r.best_index = 0;
    return r;
}

// --- Nivel de audio y ruido fuerte -----------------------------------------

static void test_audio_level_is_forwarded_unchanged(void)
{
    alert_dispatcher_on_audio_level(0);
    alert_dispatcher_on_audio_level(55);
    alert_dispatcher_on_audio_level(100);
    TEST_ASSERT_EQUAL(3, audio_level_count);
    TEST_ASSERT_EQUAL(0,   audio_levels[0]);
    TEST_ASSERT_EQUAL(55,  audio_levels[1]);
    TEST_ASSERT_EQUAL(100, audio_levels[2]);
}

// El umbral de ruido fuerte es 90: a 89 no pasa nada, a 90 se avisa y se vibra la "I".
static void test_loud_sound_threshold(void)
{
    alert_dispatcher_on_audio_level(89);
    TEST_ASSERT_EQUAL(0, notification_count);
    TEST_ASSERT_EQUAL(0, vibration_count);

    alert_dispatcher_on_audio_level(90);
    TEST_ASSERT_EQUAL(1, notification_count);
    TEST_ASSERT_EQUAL_STRING("Ruido fuerte cerca!", notifications[0]);
    TEST_ASSERT_EQUAL(1, vibration_count);
    TEST_ASSERT_EQUAL_STRING("I", vibrations[0]);
}

// Solo avisa al subir: mientras siga fuerte no repite, y vuelve a avisar tras bajar.
static void test_loud_sound_warns_only_on_the_rising_edge(void)
{
    alert_dispatcher_on_audio_level(95);
    alert_dispatcher_on_audio_level(100);
    alert_dispatcher_on_audio_level(90);
    TEST_ASSERT_EQUAL(1, notification_count);
    TEST_ASSERT_EQUAL(1, vibration_count);

    alert_dispatcher_on_audio_level(10);
    alert_dispatcher_on_audio_level(95);
    TEST_ASSERT_EQUAL(2, notification_count);
    TEST_ASSERT_EQUAL(2, vibration_count);
}

// Tras init el estado de "ruido fuerte" vuelve a cero: sin eso, un ruido fuerte que ya
// estaba sonando antes del reinicio no avisaria.
static void test_init_resets_the_loud_sound_state(void)
{
    alert_dispatcher_on_audio_level(95);
    alert_dispatcher_init(&OUTPUTS);
    alert_dispatcher_on_audio_level(95);
    TEST_ASSERT_EQUAL(2, notification_count);
}

// --- Clasificaciones -------------------------------------------------------

static void test_high_level_vibrates_the_morse_of_its_class(void)
{
    ei_wrapper_result_t r = single(catalog_label(0), 0.9f);
    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(1, vibration_count);
    TEST_ASSERT_EQUAL_STRING(catalog_morse(0), vibrations[0]);
}

static void test_medium_level_vibrates_too(void)
{
    ei_wrapper_result_t r = single(catalog_label(1), 0.65f);
    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(1, vibration_count);
    TEST_ASSERT_EQUAL_STRING(catalog_morse(1), vibrations[0]);
}

// LOW (0.55) y NONE (0.1) actualizan el icono pero no hacen vibrar.
static void test_low_and_none_levels_do_not_vibrate(void)
{
    ei_wrapper_result_t r = single(catalog_label(0), 0.55f);
    r.classifications[1] = (ei_wrapper_classification_t){ .confidence = 0.1f };
    strcpy(r.classifications[1].label, catalog_label(1));
    r.count = 2;

    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(2, icon_count);
    TEST_ASSERT_EQUAL(0, vibration_count);
}

// Una etiqueta que el catalogo no conoce no tiene morse: no vibra, pero su icono se actualiza.
static void test_unknown_class_does_not_vibrate_but_updates_its_icon(void)
{
    ei_wrapper_result_t r = single("dog-barking", 0.9f);
    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(0, vibration_count);
    TEST_ASSERT_EQUAL(1, icon_count);
    TEST_ASSERT_EQUAL_STRING("dog-barking", icons[0].label);
    TEST_ASSERT_EQUAL(ALERT_HIGH, icons[0].level);
}

// Cada clase del resultado recibe su propio nivel, en el orden en que llegan.
static void test_each_class_updates_its_icon_with_its_own_level(void)
{
    ei_wrapper_result_t r;
    memset(&r, 0, sizeof(r));
    const float confidences[3] = { 0.9f, 0.65f, 0.1f };
    for (int i = 0; i < 3; i++) {
        strcpy(r.classifications[i].label, catalog_label((size_t)i));
        r.classifications[i].confidence = confidences[i];
    }
    r.count = 3;

    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(3, icon_count);
    TEST_ASSERT_EQUAL_STRING(catalog_label(0), icons[0].label);
    TEST_ASSERT_EQUAL(ALERT_HIGH,   icons[0].level);
    TEST_ASSERT_EQUAL_STRING(catalog_label(1), icons[1].label);
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, icons[1].level);
    TEST_ASSERT_EQUAL_STRING(catalog_label(2), icons[2].label);
    TEST_ASSERT_EQUAL(ALERT_NONE,   icons[2].level);
}

// Al mensaje de inferencia llega la mejor clase (best_index), no la primera.
static void test_inference_message_carries_the_best_class(void)
{
    ei_wrapper_result_t r = single("noise", 0.2f);
    strcpy(r.classifications[1].label, "fire");
    r.classifications[1].confidence = 0.8f;
    r.count = 2;
    r.best_index = 1;

    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(1, inference_count);
    TEST_ASSERT_EQUAL_STRING("fire", inferences[0].label);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, inferences[0].confidence);
}

// Una salida sin conectar se ignora en vez de romper.
static void test_missing_outputs_are_ignored(void)
{
    alert_dispatcher_init(NULL);
    ei_wrapper_result_t r = single(catalog_label(0), 0.9f);
    alert_dispatcher_on_audio_level(95);
    alert_dispatcher_on_classification(&r);
    TEST_ASSERT_EQUAL(0, audio_level_count + notification_count + vibration_count + inference_count + icon_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_audio_level_is_forwarded_unchanged);
    RUN_TEST(test_loud_sound_threshold);
    RUN_TEST(test_loud_sound_warns_only_on_the_rising_edge);
    RUN_TEST(test_init_resets_the_loud_sound_state);
    RUN_TEST(test_high_level_vibrates_the_morse_of_its_class);
    RUN_TEST(test_medium_level_vibrates_too);
    RUN_TEST(test_low_and_none_levels_do_not_vibrate);
    RUN_TEST(test_unknown_class_does_not_vibrate_but_updates_its_icon);
    RUN_TEST(test_each_class_updates_its_icon_with_its_own_level);
    RUN_TEST(test_inference_message_carries_the_best_class);
    RUN_TEST(test_missing_outputs_are_ignored);
    return UNITY_END();
}
