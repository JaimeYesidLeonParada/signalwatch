#include "unity.h"
#include "alert_catalog.h"
#include <stdint.h>
#include <string.h>

// vibration_morse_play() copia el texto en un buffer de 32 bytes (VIBRATOR_MAX_MESSAGE_LEN).
#define MORSE_MAX_CHARS 31

void setUp(void) {}
void tearDown(void) {}

static void test_index_of_returns_the_position_of_each_class(void)
{
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        TEST_ASSERT_EQUAL_INT((int)i, alert_catalog_index_of(alert_catalog_get(i)->label));
    }
}

// Las etiquetas se comparan completas: ni un prefijo ni la cadena vacia son una clase.
static void test_index_of_unknown_label_returns_minus_one(void)
{
    TEST_ASSERT_EQUAL_INT(-1, alert_catalog_index_of("unknown"));
    TEST_ASSERT_EQUAL_INT(-1, alert_catalog_index_of(""));
    TEST_ASSERT_EQUAL_INT(-1, alert_catalog_index_of("fir"));
}

static void test_get_out_of_range_returns_null(void)
{
    TEST_ASSERT_NULL(alert_catalog_get(alert_catalog_count()));
    TEST_ASSERT_NULL(alert_catalog_get(SIZE_MAX));
}

// Si dos clases comparten etiqueta, index_of siempre devolveria la primera.
static void test_labels_are_unique(void)
{
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        for (size_t j = i + 1; j < alert_catalog_count(); j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(0, strcmp(alert_catalog_get(i)->label, alert_catalog_get(j)->label),
                                          "etiqueta repetida en el catalogo");
        }
    }
}

// Una fila incompleta no falla al compilar: aparece como un icono o texto vacio en el reloj.
static void test_every_class_has_all_its_fields(void)
{
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        const alert_class_t *c = alert_catalog_get(i);
        TEST_ASSERT_NOT_NULL(c->label);
        TEST_ASSERT_NOT_NULL(c->morse);
        TEST_ASSERT_NOT_NULL(c->glyph);
        TEST_ASSERT_TRUE(c->label[0] != '\0');
        TEST_ASSERT_TRUE(c->morse[0] != '\0');
        TEST_ASSERT_TRUE(c->glyph[0] != '\0');
        for (int level = 0; level < 3; level++) {
            TEST_ASSERT_NOT_NULL(c->msg[level]);
            TEST_ASSERT_TRUE(c->msg[level][0] != '\0');
        }
    }
}

// El vibrador ignora en silencio los caracteres que no sean A-Z, 0-9 o espacio,
// y corta el texto a 31 caracteres.
static void test_morse_text_is_playable(void)
{
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        const char *morse = alert_catalog_get(i)->morse;
        TEST_ASSERT_NOT_NULL(morse);
        TEST_ASSERT_TRUE(strlen(morse) <= MORSE_MAX_CHARS);
        for (const char *p = morse; *p != '\0'; p++) {
            int valid = (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
                        (*p >= '0' && *p <= '9') || *p == ' ';
            TEST_ASSERT_TRUE_MESSAGE(valid, "caracter sin codigo morse");
        }
    }
}

// La fuente de la pantalla no tiene tildes ni "¡" (regla de CLAUDE.md): el texto debe ser ASCII.
// El glifo del icono queda fuera a proposito, porque es UTF-8 de la fuente fa_icons_32.
static void test_screen_text_is_ascii(void)
{
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        for (int level = 0; level < 3; level++) {
            TEST_ASSERT_NOT_NULL(alert_catalog_get(i)->msg[level]);
            for (const unsigned char *p = (const unsigned char *)alert_catalog_get(i)->msg[level]; *p != '\0'; p++) {
                TEST_ASSERT_TRUE_MESSAGE(*p < 0x80, "caracter no ASCII en un texto de pantalla");
            }
        }
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_index_of_returns_the_position_of_each_class);
    RUN_TEST(test_index_of_unknown_label_returns_minus_one);
    RUN_TEST(test_get_out_of_range_returns_null);
    RUN_TEST(test_labels_are_unique);
    RUN_TEST(test_every_class_has_all_its_fields);
    RUN_TEST(test_morse_text_is_playable);
    RUN_TEST(test_screen_text_is_ascii);
    return UNITY_END();
}
