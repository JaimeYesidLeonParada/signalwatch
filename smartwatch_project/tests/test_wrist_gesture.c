#include "unity.h"
#include "wrist_gesture.h"

void setUp(void) {}
void tearDown(void) {}

// Poses de ejemplo (ax, ay, az en milli-g, con modulo cercano a 1 g).
#define HANGING   0, 1000, 0      // brazo caido: la pantalla mira de lado
#define LOOKING   0, 400, 900     // pantalla hacia arriba, inclinada hacia la cara
#define MID_RAISE 0, 800, 400     // a medio camino: ni caido ni mirando

// Alimenta n muestras iguales y devuelve cuantas veces se disparo el gesto.
static int feed_n(wrist_gesture_t *g, int n, int ax, int ay, int az)
{
    int fired = 0;
    for (int i = 0; i < n; i++) {
        if (wrist_gesture_feed(g, ax, ay, az)) fired++;
    }
    return fired;
}

// Muestras necesarias para mantener la pose el tiempo que pide el gesto.
#define HOLD_SAMPLES ((WRIST_HOLD_MS + WRIST_GESTURE_SAMPLE_PERIOD_MS - 1) / WRIST_GESTURE_SAMPLE_PERIOD_MS)

static void test_raising_the_wrist_and_holding_fires_the_gesture(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    TEST_ASSERT_EQUAL(1, feed_n(&g, HOLD_SAMPLES, LOOKING));
}

static void test_the_gesture_fires_only_once_while_the_pose_is_held(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    TEST_ASSERT_EQUAL(1, feed_n(&g, 100, LOOKING));
}

// Un reloj que ya estaba mirando hacia arriba (sobre una mesa) no es un gesto.
static void test_a_watch_that_never_hung_does_not_fire(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    TEST_ASSERT_EQUAL(0, feed_n(&g, 100, LOOKING));
}

static void test_a_pose_held_less_than_the_hold_time_does_not_fire(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    TEST_ASSERT_EQUAL(0, feed_n(&g, HOLD_SAMPLES - 1, LOOKING));
}

// Subir el brazo demasiado despacio (mas del tiempo maximo) no cuenta como "levantar para mirar".
static void test_raising_too_slowly_does_not_fire(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    feed_n(&g, WRIST_RAISE_WINDOW_MS / WRIST_GESTURE_SAMPLE_PERIOD_MS, MID_RAISE);
    TEST_ASSERT_EQUAL(0, feed_n(&g, HOLD_SAMPLES, LOOKING));
}

// Mientras el reloj se sacude (modulo muy lejos de 1 g) no se decide nada.
static void test_a_shaking_watch_does_not_fire(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    TEST_ASSERT_EQUAL(0, feed_n(&g, 50, 0, 1200, 1500));
}

// Tras disparar hay que volver a bajar el brazo para que dispare otra vez.
static void test_it_fires_again_only_after_the_arm_is_lowered(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    feed_n(&g, 20, LOOKING);
    TEST_ASSERT_EQUAL(0, feed_n(&g, 20, LOOKING));
    feed_n(&g, 10, HANGING);
    TEST_ASSERT_EQUAL(1, feed_n(&g, HOLD_SAMPLES, LOOKING));
}

// Con la pantalla hacia abajo tambien cuenta como brazo caido.
static void test_a_face_down_watch_counts_as_lowered(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, 0, 0, -1000);
    TEST_ASSERT_EQUAL(1, feed_n(&g, HOLD_SAMPLES, LOOKING));
}

// init borra lo anterior: un brazo caido visto antes de reiniciar no vale.
static void test_init_forgets_the_previous_lowered_arm(void)
{
    wrist_gesture_t g;
    wrist_gesture_init(&g);
    feed_n(&g, 10, HANGING);
    wrist_gesture_init(&g);
    TEST_ASSERT_EQUAL(0, feed_n(&g, HOLD_SAMPLES, LOOKING));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_raising_the_wrist_and_holding_fires_the_gesture);
    RUN_TEST(test_the_gesture_fires_only_once_while_the_pose_is_held);
    RUN_TEST(test_a_watch_that_never_hung_does_not_fire);
    RUN_TEST(test_a_pose_held_less_than_the_hold_time_does_not_fire);
    RUN_TEST(test_raising_too_slowly_does_not_fire);
    RUN_TEST(test_a_shaking_watch_does_not_fire);
    RUN_TEST(test_it_fires_again_only_after_the_arm_is_lowered);
    RUN_TEST(test_a_face_down_watch_counts_as_lowered);
    RUN_TEST(test_init_forgets_the_previous_lowered_arm);
    return UNITY_END();
}
