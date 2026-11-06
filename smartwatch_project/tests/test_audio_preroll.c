#include "unity.h"
#include "audio_preroll.h"
#include <string.h>

static audio_preroll_t preroll;
static int16_t out[AUDIO_PREROLL_CHUNKS * AUDIO_CHUNK_SAMPLES];

void setUp(void)
{
    memset(&preroll, 0, sizeof(preroll));
    memset(out, 0x55, sizeof(out));
}

void tearDown(void) {}

// Empuja un trozo cuyas muestras valen todas value.
static void push_constant(int16_t value)
{
    int16_t chunk[AUDIO_CHUNK_SAMPLES];
    for (int i = 0; i < AUDIO_CHUNK_SAMPLES; i++) chunk[i] = value;
    audio_preroll_push(&preroll, chunk);
}

// Comprueba que el trozo number de la salida tiene todas sus muestras iguales a value.
static void assert_out_chunk(int number, int16_t value)
{
    for (int i = 0; i < AUDIO_CHUNK_SAMPLES; i++) {
        TEST_ASSERT_EQUAL_INT16_MESSAGE(value, out[number * AUDIO_CHUNK_SAMPLES + i], "trozo en orden equivocado");
    }
}

// Con el buffer justo lleno, la salida va del primer trozo recibido al ultimo.
static void test_output_goes_from_oldest_to_newest(void)
{
    for (int v = 1; v <= AUDIO_PREROLL_CHUNKS; v++) push_constant((int16_t)v);
    audio_preroll_copy_out(&preroll, out);
    for (int c = 0; c < AUDIO_PREROLL_CHUNKS; c++) assert_out_chunk(c, (int16_t)(c + 1));
}

// Tras dar la vuelta, los trozos nuevos reemplazan a los mas viejos y el orden se mantiene:
// con 6 trozos recibidos quedan el 3, 4, 5 y 6.
static void test_new_chunks_replace_the_oldest_after_wrapping(void)
{
    for (int v = 1; v <= 6; v++) push_constant((int16_t)v);
    audio_preroll_copy_out(&preroll, out);
    assert_out_chunk(0, 3);
    assert_out_chunk(1, 4);
    assert_out_chunk(2, 5);
    assert_out_chunk(3, 6);
}

// Al arrancar, los trozos que aun no llegaron son silencio y van primero (son los mas viejos).
static void test_missing_chunks_are_silence_and_come_first(void)
{
    push_constant(7);
    push_constant(8);
    audio_preroll_copy_out(&preroll, out);
    assert_out_chunk(0, 0);
    assert_out_chunk(1, 0);
    assert_out_chunk(2, 7);
    assert_out_chunk(3, 8);
}

// Cada muestra llega a su lugar, sin mezclar ni desplazar dentro del trozo.
static void test_samples_inside_a_chunk_are_kept_in_order(void)
{
    int16_t ramp[AUDIO_CHUNK_SAMPLES];
    for (int i = 0; i < AUDIO_CHUNK_SAMPLES; i++) ramp[i] = (int16_t)(i - 500);
    for (int k = 0; k < AUDIO_PREROLL_CHUNKS; k++) audio_preroll_push(&preroll, ramp);

    audio_preroll_copy_out(&preroll, out);
    for (int c = 0; c < AUDIO_PREROLL_CHUNKS; c++) {
        for (int i = 0; i < AUDIO_CHUNK_SAMPLES; i++) {
            TEST_ASSERT_EQUAL_INT16(ramp[i], out[c * AUDIO_CHUNK_SAMPLES + i]);
        }
    }
}

// Copiar no consume nada: dos copias seguidas dan lo mismo y el buffer sigue funcionando despues.
static void test_copy_out_does_not_consume_the_chunks(void)
{
    for (int v = 1; v <= AUDIO_PREROLL_CHUNKS; v++) push_constant((int16_t)v);

    static int16_t first[AUDIO_PREROLL_CHUNKS * AUDIO_CHUNK_SAMPLES];
    audio_preroll_copy_out(&preroll, first);
    audio_preroll_copy_out(&preroll, out);
    TEST_ASSERT_EQUAL_INT16_ARRAY(first, out, AUDIO_PREROLL_CHUNKS * AUDIO_CHUNK_SAMPLES);

    push_constant(5);
    audio_preroll_copy_out(&preroll, out);
    assert_out_chunk(0, 2);
    assert_out_chunk(3, 5);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_output_goes_from_oldest_to_newest);
    RUN_TEST(test_new_chunks_replace_the_oldest_after_wrapping);
    RUN_TEST(test_missing_chunks_are_silence_and_come_first);
    RUN_TEST(test_samples_inside_a_chunk_are_kept_in_order);
    RUN_TEST(test_copy_out_does_not_consume_the_chunks);
    return UNITY_END();
}
