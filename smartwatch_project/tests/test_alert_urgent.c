#include "unity.h"
#include "alert_urgent.h"
#include <stddef.h>

void setUp(void) {}
void tearDown(void) {}

static void test_alert_class_above_90_percent_is_urgent(void)
{
    TEST_ASSERT_TRUE(alert_is_urgent("ambulance", 0.95f));
    TEST_ASSERT_TRUE(alert_is_urgent("fire", 0.91f));
}

static void test_alert_class_at_or_below_90_percent_is_not_urgent(void)
{
    TEST_ASSERT_FALSE(alert_is_urgent("ambulance", 0.90f));
    TEST_ASSERT_FALSE(alert_is_urgent("ambulance", 0.60f));
    TEST_ASSERT_FALSE(alert_is_urgent("ambulance", 0.0f));
}

// Una etiqueta que no esta en el catalogo (ruido de fondo, silencio) nunca interrumpe.
static void test_label_outside_the_catalog_is_never_urgent(void)
{
    TEST_ASSERT_FALSE(alert_is_urgent("noise", 0.99f));
    TEST_ASSERT_FALSE(alert_is_urgent("", 0.99f));
}

static void test_null_label_is_not_urgent(void)
{
    TEST_ASSERT_FALSE(alert_is_urgent(NULL, 0.99f));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_alert_class_above_90_percent_is_urgent);
    RUN_TEST(test_alert_class_at_or_below_90_percent_is_not_urgent);
    RUN_TEST(test_label_outside_the_catalog_is_never_urgent);
    RUN_TEST(test_null_label_is_not_urgent);
    return UNITY_END();
}
