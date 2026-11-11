#include "unity.h"
#include "base64_encode.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// Vectores de prueba del RFC 4648.
static void encode_and_check(const char *input, const char *expected)
{
    char out[64];
    size_t n = base64_encode((const uint8_t *)input, strlen(input), out);
    TEST_ASSERT_EQUAL_STRING(expected, out);
    TEST_ASSERT_EQUAL(strlen(expected), n);
}

static void test_empty_input_gives_an_empty_string(void)
{
    encode_and_check("", "");
}

static void test_three_bytes_need_no_padding(void)
{
    encode_and_check("foo", "Zm9v");
}

static void test_one_leftover_byte_is_padded_with_two_equals(void)
{
    encode_and_check("f", "Zg==");
    encode_and_check("foob", "Zm9vYg==");
}

static void test_two_leftover_bytes_are_padded_with_one_equal(void)
{
    encode_and_check("fo", "Zm8=");
    encode_and_check("fooba", "Zm9vYmE=");
}

static void test_the_last_two_alphabet_characters_are_plus_and_slash(void)
{
    const uint8_t in[3] = { 0xFF, 0xFE, 0xFD };   // 111111 111111 111011 111101
    char out[8];
    base64_encode(in, 3, out);
    TEST_ASSERT_EQUAL_STRING("//79", out);
}

static void test_encoded_size_macro_covers_the_output_and_the_terminator(void)
{
    TEST_ASSERT_EQUAL(1, BASE64_ENCODED_SIZE(0));
    TEST_ASSERT_EQUAL(5, BASE64_ENCODED_SIZE(3));
    TEST_ASSERT_EQUAL(9, BASE64_ENCODED_SIZE(4));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_input_gives_an_empty_string);
    RUN_TEST(test_three_bytes_need_no_padding);
    RUN_TEST(test_one_leftover_byte_is_padded_with_two_equals);
    RUN_TEST(test_two_leftover_bytes_are_padded_with_one_equal);
    RUN_TEST(test_the_last_two_alphabet_characters_are_plus_and_slash);
    RUN_TEST(test_encoded_size_macro_covers_the_output_and_the_terminator);
    return UNITY_END();
}
