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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_decodes_a_known_frame);
    RUN_TEST(test_control_bits_are_not_part_of_the_time);
    RUN_TEST(test_range_limits_are_accepted);
    RUN_TEST(test_out_of_range_fields_are_rejected);
    RUN_TEST(test_invalid_bcd_digits_are_rejected);
    RUN_TEST(test_blank_chip_frames_are_rejected);
    return UNITY_END();
}
