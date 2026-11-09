#include "unity.h"
#include "rtc_codec.h"
#include <string.h>

// Orden de los registros a partir de 0x04: segundos, minutos, horas, dia, dia de semana, mes, anio.
enum { SEC, MIN, HR, DAY, DOW, MONTH, YEAR };

// 14:35:42, miercoles (3), 17 de noviembre, anio 25 (todo en BCD).
static const uint8_t VALID_FRAME[RTC_RAW_LEN] = { 0x42, 0x35, 0x14, 0x17, 0x03, 0x11, 0x25 };

void setUp(void) {}
void tearDown(void) {}

// Decodifica la trama de referencia con un solo byte cambiado.
static bool decode_with(int field, uint8_t value, rtc_time_t *out)
{
    uint8_t raw[RTC_RAW_LEN];
    memcpy(raw, VALID_FRAME, sizeof(raw));
    raw[field] = value;
    return rtc_decode_registers(raw, out);
}

static void test_decodes_a_known_frame(void)
{
    rtc_time_t t;
    TEST_ASSERT_TRUE(rtc_decode_registers(VALID_FRAME, &t));
    TEST_ASSERT_EQUAL_UINT8(42, t.sec);
    TEST_ASSERT_EQUAL_UINT8(35, t.min);
    TEST_ASSERT_EQUAL_UINT8(14, t.hr);
    TEST_ASSERT_EQUAL_UINT8(17, t.day);
    TEST_ASSERT_EQUAL_UINT8(3,  t.dow);
    TEST_ASSERT_EQUAL_UINT8(11, t.month);
    TEST_ASSERT_EQUAL_UINT8(25, t.year);
}

// Los bits altos de cada registro son banderas del chip (por ejemplo "oscilador parado" en los
// segundos), no parte de la hora.
static void test_control_bits_are_not_part_of_the_time(void)
{
    const uint8_t with_flags[RTC_RAW_LEN] = {
        0x80 | 0x42,   // segundos: bit 7 = bandera del oscilador
        0x80 | 0x35,
        0xC0 | 0x14,
        0xC0 | 0x17,
        0xC0 | 0x03,
        0xE0 | 0x11,
        0x25,
    };
    rtc_time_t t;
    TEST_ASSERT_TRUE(rtc_decode_registers(with_flags, &t));
    TEST_ASSERT_EQUAL_UINT8(42, t.sec);
    TEST_ASSERT_EQUAL_UINT8(35, t.min);
    TEST_ASSERT_EQUAL_UINT8(14, t.hr);
    TEST_ASSERT_EQUAL_UINT8(17, t.day);
    TEST_ASSERT_EQUAL_UINT8(3,  t.dow);
    TEST_ASSERT_EQUAL_UINT8(11, t.month);
}

// 00:00 del 1 de enero (domingo) y 23:59 del 31 de diciembre (sabado): los bordes validos se aceptan.
static void test_range_limits_are_accepted(void)
{
    const uint8_t first[RTC_RAW_LEN] = { 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 };
    const uint8_t last[RTC_RAW_LEN]  = { 0x59, 0x59, 0x23, 0x31, 0x06, 0x12, 0x99 };
    rtc_time_t t;
    TEST_ASSERT_TRUE(rtc_decode_registers(first, &t));
    TEST_ASSERT_TRUE(rtc_decode_registers(last, &t));
    TEST_ASSERT_EQUAL_UINT8(23, t.hr);
    TEST_ASSERT_EQUAL_UINT8(59, t.min);
    TEST_ASSERT_EQUAL_UINT8(31, t.day);
    TEST_ASSERT_EQUAL_UINT8(12, t.month);
}

// Cada campo fuera de rango por separado: el reloj no debe usar esa hora.
static void test_out_of_range_fields_are_rejected(void)
{
    rtc_time_t t;
    TEST_ASSERT_FALSE_MESSAGE(decode_with(HR,    0x24, &t), "hora 24");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(MIN,   0x60, &t), "minuto 60");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(DOW,   0x07, &t), "dia de semana 7");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(DAY,   0x00, &t), "dia 0");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(DAY,   0x32, &t), "dia 32");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(MONTH, 0x00, &t), "mes 0");
    TEST_ASSERT_FALSE_MESSAGE(decode_with(MONTH, 0x13, &t), "mes 13");
}

// Los digitos BCD invalidos (nibble > 9) dan valores fuera de rango y tambien se rechazan.
static void test_invalid_bcd_digits_are_rejected(void)
{
    rtc_time_t t;
    TEST_ASSERT_FALSE(decode_with(MIN, 0x6F, &t));   // 6*10 + 15 = 75
    TEST_ASSERT_FALSE(decode_with(HR,  0x2F, &t));   // 2*10 + 15 = 35
}

// Chip sin configurar o sin pila: todo ceros o todo unos. Ninguna de las dos es una hora valida.
static void test_blank_chip_frames_are_rejected(void)
{
    uint8_t zeros[RTC_RAW_LEN], ones[RTC_RAW_LEN];
    memset(zeros, 0x00, sizeof(zeros));
    memset(ones,  0xFF, sizeof(ones));
    rtc_time_t t;
    TEST_ASSERT_FALSE(rtc_decode_registers(zeros, &t));
    TEST_ASSERT_FALSE(rtc_decode_registers(ones, &t));
}

// ---- rtc_encode_registers ----

// 2025-11-17 (lunes=1) 14:35:42
static wall_time_t sample_time(void)
{
    wall_time_t t = { .year = 2025, .month = 11, .day = 17, .dow = 1,
                      .hour = 14, .minute = 35, .second = 42 };
    return t;
}

static void test_encodes_a_known_time_as_bcd(void)
{
    wall_time_t t = sample_time();
    uint8_t raw[RTC_RAW_LEN];
    TEST_ASSERT_TRUE(rtc_encode_registers(&t, raw));
    const uint8_t expected[RTC_RAW_LEN] = { 0x42, 0x35, 0x14, 0x17, 0x01, 0x11, 0x25 };
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, raw, RTC_RAW_LEN);
}

// Lo que se escribe es lo que se lee de vuelta.
static void test_encoded_time_decodes_to_the_same_time(void)
{
    wall_time_t t = sample_time();
    uint8_t raw[RTC_RAW_LEN];
    rtc_time_t back;
    TEST_ASSERT_TRUE(rtc_encode_registers(&t, raw));
    TEST_ASSERT_TRUE(rtc_decode_registers(raw, &back));
    TEST_ASSERT_EQUAL_UINT8(42, back.sec);
    TEST_ASSERT_EQUAL_UINT8(35, back.min);
    TEST_ASSERT_EQUAL_UINT8(14, back.hr);
    TEST_ASSERT_EQUAL_UINT8(17, back.day);
    TEST_ASSERT_EQUAL_UINT8(1,  back.dow);
    TEST_ASSERT_EQUAL_UINT8(11, back.month);
    TEST_ASSERT_EQUAL_UINT8(25, back.year);
}

// Escribir segundos con el bit 7 en 0 limpia la bandera de "oscilador parado".
static void test_encoded_seconds_clear_the_oscillator_flag(void)
{
    wall_time_t t = sample_time();
    t.second = 59;
    uint8_t raw[RTC_RAW_LEN];
    TEST_ASSERT_TRUE(rtc_encode_registers(&t, raw));
    TEST_ASSERT_EQUAL_UINT8(0, raw[SEC] & 0x80);
}

static void test_encode_accepts_the_range_limits(void)
{
    wall_time_t first = { 2000, 1, 1, 0, 0, 0, 0 };
    wall_time_t last  = { 2099, 12, 31, 6, 23, 59, 59 };
    uint8_t raw[RTC_RAW_LEN];
    TEST_ASSERT_TRUE(rtc_encode_registers(&first, raw));
    TEST_ASSERT_TRUE(rtc_encode_registers(&last, raw));
    TEST_ASSERT_EQUAL_UINT8(0x99, raw[YEAR]);
}

static void test_encode_rejects_each_out_of_range_field(void)
{
    uint8_t raw[RTC_RAW_LEN];
    wall_time_t t;
    t = sample_time(); t.year = 1999;  TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "anio 1999");
    t = sample_time(); t.year = 2100;  TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "anio 2100");
    t = sample_time(); t.month = 0;    TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "mes 0");
    t = sample_time(); t.month = 13;   TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "mes 13");
    t = sample_time(); t.day = 0;      TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "dia 0");
    t = sample_time(); t.day = 32;     TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "dia 32");
    t = sample_time(); t.dow = 7;      TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "dia de semana 7");
    t = sample_time(); t.hour = 24;    TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "hora 24");
    t = sample_time(); t.minute = 60;  TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "minuto 60");
    t = sample_time(); t.second = 60;  TEST_ASSERT_FALSE_MESSAGE(rtc_encode_registers(&t, raw), "segundo 60");
}

// Una hora rechazada no debe dejar basura en el buffer que se iba a escribir.
static void test_rejected_time_leaves_the_buffer_untouched(void)
{
    wall_time_t t = sample_time();
    t.hour = 24;
    uint8_t raw[RTC_RAW_LEN];
    memset(raw, 0xAA, sizeof(raw));
    TEST_ASSERT_FALSE(rtc_encode_registers(&t, raw));
    for (int i = 0; i < RTC_RAW_LEN; i++) TEST_ASSERT_EQUAL_UINT8(0xAA, raw[i]);
}

// ---- rtc_time_to_wall_time ----

static void test_rtc_time_converts_to_wall_time(void)
{
    rtc_time_t in = { .sec = 42, .min = 35, .hr = 14, .day = 17, .dow = 1, .month = 11, .year = 25 };
    wall_time_t out;
    rtc_time_to_wall_time(&in, &out);
    TEST_ASSERT_EQUAL_UINT16(2025, out.year);
    TEST_ASSERT_EQUAL_UINT8(11, out.month);
    TEST_ASSERT_EQUAL_UINT8(17, out.day);
    TEST_ASSERT_EQUAL_UINT8(1,  out.dow);
    TEST_ASSERT_EQUAL_UINT8(14, out.hour);
    TEST_ASSERT_EQUAL_UINT8(35, out.minute);
    TEST_ASSERT_EQUAL_UINT8(42, out.second);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_rtc_time_converts_to_wall_time);
    RUN_TEST(test_encodes_a_known_time_as_bcd);
    RUN_TEST(test_encoded_time_decodes_to_the_same_time);
    RUN_TEST(test_encoded_seconds_clear_the_oscillator_flag);
    RUN_TEST(test_encode_accepts_the_range_limits);
    RUN_TEST(test_encode_rejects_each_out_of_range_field);
    RUN_TEST(test_rejected_time_leaves_the_buffer_untouched);
    RUN_TEST(test_decodes_a_known_frame);
    RUN_TEST(test_control_bits_are_not_part_of_the_time);
    RUN_TEST(test_range_limits_are_accepted);
    RUN_TEST(test_out_of_range_fields_are_rejected);
    RUN_TEST(test_invalid_bcd_digits_are_rejected);
    RUN_TEST(test_blank_chip_frames_are_rejected);
    return UNITY_END();
}
