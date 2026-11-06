#include "unity.h"
#include "audio_level.h"
#include <stdint.h>

#define N 1000   // muestras por trozo (AUDIO_CHUNK_SAMPLES)

static int16_t chunk[N];

void setUp(void) {}
void tearDown(void) {}

static void fill(int16_t value)
{
    for (int i = 0; i < N; i++) chunk[i] = value;
}

static void test_silence_has_level_zero(void)
{
    fill(0);
    TEST_ASSERT_EQUAL(0, audio_level_from_chunk(chunk, N));
}

// Promedio 600 / 20 = 30.
static void test_level_is_the_mean_amplitude_divided_by_twenty(void)
{
    fill(600);
    TEST_ASSERT_EQUAL(30, audio_level_from_chunk(chunk, N));
}

// Solo cuenta la amplitud: una onda negativa, o una que alterna, da el mismo nivel que la positiva.
static void test_negative_samples_count_like_positive_ones(void)
{
    fill(-600);
    TEST_ASSERT_EQUAL(30, audio_level_from_chunk(chunk, N));

    for (int i = 0; i < N; i++) chunk[i] = (i % 2) ? 600 : -600;
    TEST_ASSERT_EQUAL(30, audio_level_from_chunk(chunk, N));
}

// El promedio es sobre las muestras recibidas, no sobre un tamano fijo.
static void test_mean_uses_the_number_of_samples_given(void)
{
    fill(400);
    TEST_ASSERT_EQUAL(20, audio_level_from_chunk(chunk, 10));
}

// El disparo de captura es nivel >= 15: promedio 300 da 15 y 299 da 14.
static void test_level_changes_exactly_at_the_trigger_boundary(void)
{
    fill(300);
    TEST_ASSERT_EQUAL(15, audio_level_from_chunk(chunk, N));
    fill(299);
    TEST_ASSERT_EQUAL(14, audio_level_from_chunk(chunk, N));
}

static void test_level_is_clamped_to_one_hundred(void)
{
    fill(1999);
    TEST_ASSERT_EQUAL(99, audio_level_from_chunk(chunk, N));
    fill(2000);
    TEST_ASSERT_EQUAL(100, audio_level_from_chunk(chunk, N));
    fill(2500);
    TEST_ASSERT_EQUAL(100, audio_level_from_chunk(chunk, N));
    fill(INT16_MAX);
    TEST_ASSERT_EQUAL(100, audio_level_from_chunk(chunk, N));
}

// -32768 no tiene opuesto en 16 bits: una sola muestra asi (32768 / 1000 / 20 = 1) no debe dar
// un nivel negativo por desbordar al tomar el valor absoluto.
static void test_most_negative_sample_does_not_overflow(void)
{
    fill(0);
    chunk[0] = INT16_MIN;
    TEST_ASSERT_EQUAL(1, audio_level_from_chunk(chunk, N));

    fill(INT16_MIN);
    TEST_ASSERT_EQUAL(100, audio_level_from_chunk(chunk, N));
}

static void test_no_samples_has_level_zero(void)
{
    TEST_ASSERT_EQUAL(0, audio_level_from_chunk(chunk, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_silence_has_level_zero);
    RUN_TEST(test_level_is_the_mean_amplitude_divided_by_twenty);
    RUN_TEST(test_negative_samples_count_like_positive_ones);
    RUN_TEST(test_mean_uses_the_number_of_samples_given);
    RUN_TEST(test_level_changes_exactly_at_the_trigger_boundary);
    RUN_TEST(test_level_is_clamped_to_one_hundred);
    RUN_TEST(test_most_negative_sample_does_not_overflow);
    RUN_TEST(test_no_samples_has_level_zero);
    return UNITY_END();
}
