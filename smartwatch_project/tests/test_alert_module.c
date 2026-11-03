#include "unity.h"
#include "alert_module.h"

void setUp(void)
{
    alert_module_init();
}

void tearDown(void) {}

// Con una sola muestra el promedio es la propia muestra, asi que cada borde de
// umbral se prueba con una clase distinta (una ventana limpia por valor).
static void test_level_thresholds(void)
{
    TEST_ASSERT_EQUAL(ALERT_NONE,   alert_module_add_sample("a", 0.49f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    alert_module_add_sample("b", 0.50f));
    TEST_ASSERT_EQUAL(ALERT_LOW,    alert_module_add_sample("c", 0.59f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, alert_module_add_sample("d", 0.60f));
    TEST_ASSERT_EQUAL(ALERT_MEDIUM, alert_module_add_sample("e", 0.69f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   alert_module_add_sample("f", 0.70f));
    TEST_ASSERT_EQUAL(ALERT_HIGH,   alert_module_add_sample("g", 1.00f));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_level_thresholds);
    return UNITY_END();
}
