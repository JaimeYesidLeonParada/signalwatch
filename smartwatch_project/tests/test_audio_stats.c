#include "unity.h"
#include "audio_stats.h"

void setUp(void) {}
void tearDown(void) {}

static void test_silence_has_no_peak_no_rms_and_no_clipping(void)
{
    int16_t s[100] = {0};
    audio_stats_t st;
    audio_stats_compute(s, 100, &st);
    TEST_ASSERT_EQUAL(0, st.peak);
    TEST_ASSERT_EQUAL(0, st.rms);
    TEST_ASSERT_EQUAL(0, st.clipped);
}

// Una senal constante de 1000 tiene pico 1000 y valor eficaz 1000.
static void test_constant_signal_has_equal_peak_and_rms(void)
{
    int16_t s[50];
    for (int i = 0; i < 50; i++) s[i] = 1000;
    audio_stats_t st;
    audio_stats_compute(s, 50, &st);
    TEST_ASSERT_EQUAL(1000, st.peak);
    TEST_ASSERT_EQUAL(1000, st.rms);
}

// El pico mira el valor absoluto: un valle de -3000 pesa mas que una cresta de 2000.
static void test_peak_uses_the_absolute_value(void)
{
    int16_t s[4] = { 2000, -3000, 500, 0 };
    audio_stats_t st;
    audio_stats_compute(s, 4, &st);
    TEST_ASSERT_EQUAL(3000, st.peak);
}

// -32768 no cabe en un int16 positivo: no debe desbordar.
static void test_the_most_negative_sample_does_not_overflow(void)
{
    int16_t s[2] = { -32768, 0 };
    audio_stats_t st;
    audio_stats_compute(s, 2, &st);
    TEST_ASSERT_EQUAL(32768, st.peak);
}

static void test_clipped_counts_samples_at_or_above_the_clip_level(void)
{
    int16_t s[5] = { 32767, -32768, AUDIO_CLIP_LEVEL, AUDIO_CLIP_LEVEL - 1, 100 };
    audio_stats_t st;
    audio_stats_compute(s, 5, &st);
    TEST_ASSERT_EQUAL(3, st.clipped);
}

// Una onda cuadrada de +-2000 tiene RMS 2000; mitad de ceros la baja a 2000 / raiz(2) ~= 1414.
static void test_rms_averages_the_energy_over_all_samples(void)
{
    int16_t s[4] = { 2000, -2000, 0, 0 };
    audio_stats_t st;
    audio_stats_compute(s, 4, &st);
    TEST_ASSERT_EQUAL(1414, st.rms);
}

static void test_no_samples_gives_zeros(void)
{
    audio_stats_t st = { 1, 2, 3 };
    audio_stats_compute(NULL, 0, &st);
    TEST_ASSERT_EQUAL(0, st.peak);
    TEST_ASSERT_EQUAL(0, st.rms);
    TEST_ASSERT_EQUAL(0, st.clipped);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_silence_has_no_peak_no_rms_and_no_clipping);
    RUN_TEST(test_constant_signal_has_equal_peak_and_rms);
    RUN_TEST(test_peak_uses_the_absolute_value);
    RUN_TEST(test_the_most_negative_sample_does_not_overflow);
    RUN_TEST(test_clipped_counts_samples_at_or_above_the_clip_level);
    RUN_TEST(test_rms_averages_the_energy_over_all_samples);
    RUN_TEST(test_no_samples_gives_zeros);
    return UNITY_END();
}
