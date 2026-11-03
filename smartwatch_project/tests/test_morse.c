#include "unity.h"
#include "morse.h"
#include <string.h>

#define MAX_MSG_CHARS 31   // vibration_morse_play() recorta el texto a 31 caracteres
#define BUF_STEPS     (MAX_MSG_CHARS * MORSE_STEPS_PER_CHAR)

void setUp(void) {}
void tearDown(void) {}

static void assert_step(const morse_step_t *step, bool on, uint16_t ms)
{
    TEST_ASSERT_EQUAL(on, step->on);
    TEST_ASSERT_EQUAL_UINT16(ms, step->ms);
}

// Campo por campo: comparar la estructura con memcmp leeria bytes de relleno sin inicializar.
static void assert_same_steps(const morse_step_t *expected, const morse_step_t *actual, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        assert_step(&actual[i], expected[i].on, expected[i].ms);
    }
}

// --- morse_for_char --------------------------------------------------------

static void test_known_codes(void)
{
    TEST_ASSERT_EQUAL_STRING(".-",    morse_for_char('A'));
    TEST_ASSERT_EQUAL_STRING("...",   morse_for_char('S'));
    TEST_ASSERT_EQUAL_STRING("---",   morse_for_char('O'));
    TEST_ASSERT_EQUAL_STRING("--..",  morse_for_char('Z'));
    TEST_ASSERT_EQUAL_STRING("-----", morse_for_char('0'));
    TEST_ASSERT_EQUAL_STRING("----.", morse_for_char('9'));
}

static void test_lowercase_has_the_same_code_as_uppercase(void)
{
    for (char c = 'a'; c <= 'z'; c++) {
        TEST_ASSERT_EQUAL_STRING(morse_for_char(c - ('a' - 'A')), morse_for_char(c));
    }
}

// Los vecinos de los rangos validos en ASCII ('@' y '[' rodean A-Z, '/' y ':' rodean 0-9).
static void test_unsupported_characters_have_no_code(void)
{
    const char unsupported[] = { ' ', '!', '@', '[', '`', '{', '/', ':', '\0', (char)0xF1 };
    for (size_t i = 0; i < sizeof(unsupported); i++) {
        TEST_ASSERT_NULL(morse_for_char(unsupported[i]));
    }
}

// Detecta un error de tipeo en la tabla: un codigo repetido o con un simbolo que no es "." ni "-".
static void test_codes_are_unique_and_only_dots_and_dashes(void)
{
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t n = strlen(chars);
    for (size_t i = 0; i < n; i++) {
        const char *code = morse_for_char(chars[i]);
        TEST_ASSERT_NOT_NULL(code);
        TEST_ASSERT_TRUE(strlen(code) >= 1 && strlen(code) <= 5);
        TEST_ASSERT_EQUAL(strlen(code), strspn(code, ".-"));
        for (size_t j = i + 1; j < n; j++) {
            TEST_ASSERT_NOT_EQUAL(0, strcmp(code, morse_for_char(chars[j])));
        }
    }
}

// --- morse_encode ----------------------------------------------------------

static void test_dot_is_one_pulse(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(1, morse_encode("E", steps, BUF_STEPS));
    assert_step(&steps[0], true, MORSE_UNIT_MS);
}

// "A" = punto, pausa de simbolo, raya.
static void test_pause_between_symbols_of_a_letter(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(3, morse_encode("A", steps, BUF_STEPS));
    assert_step(&steps[0], true,  MORSE_UNIT_MS);
    assert_step(&steps[1], false, MORSE_SYMBOL_GAP_MS);
    assert_step(&steps[2], true,  MORSE_DASH_MS);
}

static void test_pause_between_letters(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(3, morse_encode("EE", steps, BUF_STEPS));
    assert_step(&steps[0], true,  MORSE_UNIT_MS);
    assert_step(&steps[1], false, MORSE_LETTER_GAP_MS);
    assert_step(&steps[2], true,  MORSE_UNIT_MS);
}

// El espacio reemplaza a la pausa de letra: la primera letra de una palabra no la lleva.
static void test_space_is_a_word_gap_instead_of_a_letter_gap(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(3, morse_encode("E E", steps, BUF_STEPS));
    assert_step(&steps[0], true,  MORSE_UNIT_MS);
    assert_step(&steps[1], false, MORSE_WORD_GAP_MS);
    assert_step(&steps[2], true,  MORSE_UNIT_MS);
}

// Un caracter sin codigo no deja ni vibracion ni pausa: "E!E" suena igual que "EE".
static void test_unsupported_characters_are_skipped_without_a_gap(void)
{
    morse_step_t with_junk[BUF_STEPS], clean[BUF_STEPS];
    size_t n_junk  = morse_encode("E!E", with_junk, BUF_STEPS);
    size_t n_clean = morse_encode("EE",  clean,     BUF_STEPS);
    TEST_ASSERT_EQUAL(n_clean, n_junk);
    assert_same_steps(clean, with_junk, n_clean);
}

// Si el caracter sin codigo abre la palabra, la primera letra real sigue sin pausa de letra.
static void test_unsupported_character_at_word_start_adds_no_gap(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(1, morse_encode("!E", steps, BUF_STEPS));
    assert_step(&steps[0], true, MORSE_UNIT_MS);

    TEST_ASSERT_EQUAL(3, morse_encode("E !E", steps, BUF_STEPS));
    assert_step(&steps[1], false, MORSE_WORD_GAP_MS);
    assert_step(&steps[2], true,  MORSE_UNIT_MS);
}

static void test_lowercase_text_sounds_like_uppercase(void)
{
    morse_step_t lower[BUF_STEPS], upper[BUF_STEPS];
    size_t n_lower = morse_encode("sos", lower, BUF_STEPS);
    size_t n_upper = morse_encode("SOS", upper, BUF_STEPS);
    TEST_ASSERT_EQUAL(n_upper, n_lower);
    assert_same_steps(upper, lower, n_upper);
}

static void test_empty_text_has_no_steps(void)
{
    morse_step_t steps[BUF_STEPS];
    TEST_ASSERT_EQUAL(0, morse_encode("", steps, BUF_STEPS));
}

// SOS en unidades de tiempo: S=5 (1+1+1+1+1), O=11 (3+1+3+1+3), S=5, mas dos pausas de letra de 3 = 27.
// Fija las proporciones del morse estandar (punto 1, raya 3, pausa de simbolo 1, de letra 3).
static void test_sos_follows_standard_morse_ratios(void)
{
    morse_step_t steps[BUF_STEPS];
    size_t n = morse_encode("SOS", steps, BUF_STEPS);
    TEST_ASSERT_EQUAL(17, n);

    uint32_t total_ms = 0;
    for (size_t i = 0; i < n; i++) total_ms += steps[i].ms;
    TEST_ASSERT_EQUAL_UINT32(27 * MORSE_UNIT_MS, total_ms);
}

// Si el buffer de pasos se llena, se corta ahi sin escribir de mas.
static void test_encode_never_writes_past_max_steps(void)
{
    morse_step_t steps[12];
    memset(steps, 0xAA, sizeof(steps));
    size_t n = morse_encode("0123456789", steps, 7);
    TEST_ASSERT_EQUAL(7, n);
    for (int i = 7; i < 12; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xAA, ((const uint8_t *)&steps[i])[0]);
        TEST_ASSERT_EQUAL_HEX8(0xAA, ((const uint8_t *)&steps[i])[1]);
    }
}

// El peor caso (31 digitos de 5 simbolos) tiene que caber en el buffer que usa vibrator_task.
static void test_worst_case_message_fits_the_step_buffer(void)
{
    morse_step_t steps[BUF_STEPS];
    char text[MAX_MSG_CHARS + 1];
    memset(text, '0', MAX_MSG_CHARS);
    text[MAX_MSG_CHARS] = '\0';

    size_t n = morse_encode(text, steps, BUF_STEPS);
    // 31 digitos * (5 pulsos + 4 pausas de simbolo) + 30 pausas de letra
    TEST_ASSERT_EQUAL(MAX_MSG_CHARS * 9 + (MAX_MSG_CHARS - 1), n);
    TEST_ASSERT_TRUE(n <= BUF_STEPS);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_known_codes);
    RUN_TEST(test_lowercase_has_the_same_code_as_uppercase);
    RUN_TEST(test_unsupported_characters_have_no_code);
    RUN_TEST(test_codes_are_unique_and_only_dots_and_dashes);
    RUN_TEST(test_dot_is_one_pulse);
    RUN_TEST(test_pause_between_symbols_of_a_letter);
    RUN_TEST(test_pause_between_letters);
    RUN_TEST(test_space_is_a_word_gap_instead_of_a_letter_gap);
    RUN_TEST(test_unsupported_characters_are_skipped_without_a_gap);
    RUN_TEST(test_unsupported_character_at_word_start_adds_no_gap);
    RUN_TEST(test_lowercase_text_sounds_like_uppercase);
    RUN_TEST(test_empty_text_has_no_steps);
    RUN_TEST(test_sos_follows_standard_morse_ratios);
    RUN_TEST(test_encode_never_writes_past_max_steps);
    RUN_TEST(test_worst_case_message_fits_the_step_buffer);
    return UNITY_END();
}
