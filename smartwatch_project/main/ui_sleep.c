#include "ui_sleep.h"
#include "screen_policy.h"

// Cada cuanto se revisa si hay que apagar. Tambien es la precision con la que se cuenta la
// inactividad, de sobra para un apagado a los 30 s.
#define CHECK_PERIOD_MS 250

static screen_policy_t policy;
static display_power_t display_power;
static ui_sleep_cb_t sleep_cb;
static lv_display_t *disp;
// Objeto invisible que tapa toda la pantalla mientras esta apagada: recibe el toque que la
// despierta para que no llegue a los botones de debajo.
static lv_obj_t *shield;

static void wake(void)
{
    lv_obj_add_flag(shield, LV_OBJ_FLAG_HIDDEN);
    if (display_power != NULL) {
        display_power(true);
    }
}

bool ui_sleep_activity(void)
{
    bool woke = screen_policy_activity(&policy, lv_tick_get());
    if (woke) {
        wake();
    }
    return woke;
}

// Toque con la pantalla apagada: solo despierta. El resto de ese toque (soltar, deslizar) se
// ignora.
static void on_shield_pressed(lv_event_t *e)
{
    (void)e;
    lv_indev_wait_release(lv_indev_active());
    ui_sleep_activity();
}

static void on_check(lv_timer_t *timer)
{
    (void)timer;
    if (display_power == NULL || !screen_policy_is_on(&policy)) {
        return;
    }
    uint32_t now = lv_tick_get();
    // LVGL lleva la cuenta del tiempo desde el ultimo toque: si es reciente, hubo actividad.
    if (lv_display_get_inactive_time(disp) < 2 * CHECK_PERIOD_MS) {
        screen_policy_activity(&policy, now);
    }
    if (screen_policy_tick(&policy, now)) {
        lv_obj_remove_flag(shield, LV_OBJ_FLAG_HIDDEN);
        display_power(false);
        if (sleep_cb != NULL) {
            sleep_cb();
        }
    }
}

void ui_sleep_create(lv_display_t *display, ui_sleep_cb_t on_sleep)
{
    disp = display;
    sleep_cb = on_sleep;
    screen_policy_init(&policy, SCREEN_POLICY_DEFAULT_TIMEOUT_MS, lv_tick_get());

    // En la capa superior: queda por encima de cualquier pantalla y LVGL le da el toque primero.
    shield = lv_obj_create(lv_display_get_layer_top(display));
    lv_obj_remove_style_all(shield);
    lv_obj_set_size(shield, lv_pct(100), lv_pct(100));
    lv_obj_add_flag(shield, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(shield, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(shield, on_shield_pressed, LV_EVENT_PRESSED, NULL);

    lv_timer_create(on_check, CHECK_PERIOD_MS, NULL);
}

void ui_sleep_set_power(display_power_t power)
{
    display_power = power;
}
