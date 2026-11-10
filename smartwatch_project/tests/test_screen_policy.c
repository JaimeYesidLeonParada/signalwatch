#include "unity.h"
#include "screen_policy.h"

void setUp(void) {}
void tearDown(void) {}

static void test_screen_starts_on(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    TEST_ASSERT_TRUE(screen_policy_is_on(&p));
}

static void test_screen_stays_on_before_the_timeout(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 29999));
    TEST_ASSERT_TRUE(screen_policy_is_on(&p));
}

static void test_screen_turns_off_when_the_timeout_is_reached(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    TEST_ASSERT_TRUE(screen_policy_tick(&p, 30000));
    TEST_ASSERT_FALSE(screen_policy_is_on(&p));
}

// Solo el tick que apaga avisa: los siguientes no deben apagar el backlight otra vez.
static void test_tick_reports_the_turn_off_only_once(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    screen_policy_tick(&p, 30000);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 31000));
    TEST_ASSERT_FALSE(screen_policy_is_on(&p));
}

static void test_activity_restarts_the_countdown(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    screen_policy_activity(&p, 20000);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 49999));
    TEST_ASSERT_TRUE(screen_policy_tick(&p, 50000));
}

static void test_activity_while_on_does_not_report_a_wake(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    TEST_ASSERT_FALSE(screen_policy_activity(&p, 1000));
}

static void test_activity_while_off_turns_the_screen_on_and_reports_the_wake(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    screen_policy_tick(&p, 30000);
    TEST_ASSERT_TRUE(screen_policy_activity(&p, 40000));
    TEST_ASSERT_TRUE(screen_policy_is_on(&p));
}

// Despues de despertar la cuenta empieza de nuevo desde ese momento.
static void test_countdown_after_a_wake_starts_from_the_wake(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0);
    screen_policy_tick(&p, 30000);
    screen_policy_activity(&p, 40000);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 69999));
    TEST_ASSERT_TRUE(screen_policy_tick(&p, 70000));
}

static void test_timeout_zero_never_turns_the_screen_off(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 0, 0);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 0xFFFFFFFFu));
    TEST_ASSERT_TRUE(screen_policy_is_on(&p));
}

// El contador de milisegundos de 32 bits da la vuelta cada ~49 dias; no debe apagar antes de hora.
static void test_counter_wrap_around_does_not_turn_off_early(void)
{
    screen_policy_t p;
    screen_policy_init(&p, 30000, 0xFFFFFFFFu - 5000u);
    TEST_ASSERT_FALSE(screen_policy_tick(&p, 10000));   // pasaron ~15 s
    TEST_ASSERT_TRUE(screen_policy_tick(&p, 25000));    // pasaron ~30 s
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_screen_starts_on);
    RUN_TEST(test_screen_stays_on_before_the_timeout);
    RUN_TEST(test_screen_turns_off_when_the_timeout_is_reached);
    RUN_TEST(test_tick_reports_the_turn_off_only_once);
    RUN_TEST(test_activity_restarts_the_countdown);
    RUN_TEST(test_activity_while_on_does_not_report_a_wake);
    RUN_TEST(test_activity_while_off_turns_the_screen_on_and_reports_the_wake);
    RUN_TEST(test_countdown_after_a_wake_starts_from_the_wake);
    RUN_TEST(test_timeout_zero_never_turns_the_screen_off);
    RUN_TEST(test_counter_wrap_around_does_not_turn_off_early);
    return UNITY_END();
}
