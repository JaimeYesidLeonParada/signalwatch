#include "ui.h"
#include "ui_clock.h"
#include "ui_battery.h"
#include "ui_notification.h"
#include "ui_vu.h"
#include "ui_alerts.h"
#include "ui_settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>
#include "esp_lvgl_port.h"

static lv_disp_t * disp;
// Dos pantallas: la principal (reloj y alertas) y la de ajustes. Los mensajes siempre se
// dibujan en main_scr, este o no visible, para que no aparezcan widgets en la de ajustes.
static lv_obj_t * main_scr;
static lv_obj_t * settings_scr;

typedef enum {
    UI_MSG_CLOCK,
    UI_MSG_BATTERY,
    UI_MSG_INFERENCE,
    UI_MSG_AUDIO_LEVEL,
    UI_MSG_ALERT_ICON,
    UI_MSG_NOTIFICATION,
} ui_msg_type_t;

typedef struct {
    ui_msg_type_t type;
    union {
        struct { int hour, min, dow, day, month; bool valid; } clock;
        struct { int bat_pct; bool is_charging; } battery;
        struct { char label[32]; float confidence; } inference;
        struct { int level; } audio_level;
        struct { char label[32]; int level; } alert_icon;
        struct { char notification[48];} notification;
    } data;
} ui_msg_t;

static QueueHandle_t ui_queue = NULL;

// Unico lugar que dibuja: se llama desde ui_task con el lock de LVGL tomado.
static void ui_handle_msg(const ui_msg_t *msg)
{
    lv_obj_t * scr = main_scr;

    switch (msg->type) {
        case UI_MSG_CLOCK:
        ui_clock_update(scr, msg->data.clock.hour, msg->data.clock.min, msg->data.clock.dow,
                        msg->data.clock.day, msg->data.clock.month, msg->data.clock.valid);
        break;

        case UI_MSG_BATTERY:
        ui_battery_update(scr, msg->data.battery.bat_pct, msg->data.battery.is_charging);
        break;

        case UI_MSG_INFERENCE:
        ui_notification_show_inference(scr, msg->data.inference.label, msg->data.inference.confidence);
        break;

        case UI_MSG_AUDIO_LEVEL:
        ui_vu_update(msg->data.audio_level.level);
        break;

        case UI_MSG_ALERT_ICON:
        ui_alerts_update(scr, msg->data.alert_icon.label, msg->data.alert_icon.level);
        break;

        case UI_MSG_NOTIFICATION:
        ui_notification_show_text(scr, msg->data.notification.notification);
        break;
    }
}

static void ui_task(void *arg)
{
    ui_msg_t msg;
    while (1) {
        if (xQueueReceive(ui_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (lvgl_port_lock(-1)) {
                ui_handle_msg(&msg);
                lvgl_port_unlock();
            }
        }
    }
}

// Deslizar el dedo hacia la izquierda en la principal abre los ajustes; hacia la derecha
// en los ajustes vuelve. La pantalla nueva entra por el lado contrario al del dedo.
static void on_swipe(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    lv_obj_t *target = lv_event_get_target(e);

    if (target == main_scr && dir == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_active());
        lv_screen_load_anim(settings_scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
    } else if (target == settings_scr && dir == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_active());
        lv_screen_load_anim(main_scr, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
    }
}

void ui_start(lv_disp_t *display)
{
    disp = display;
    main_scr = lv_disp_get_scr_act(disp);
    lv_obj_t * scr = main_scr;
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    ui_alerts_create(scr, ui_vu_create(scr));

    // Sin scroll en las pantallas: si no, el dedo las desplaza en vez de dar un gesto.
    lv_obj_remove_flag(main_scr, LV_OBJ_FLAG_SCROLLABLE);
    settings_scr = ui_settings_create();
    lv_obj_add_event_cb(main_scr, on_swipe, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(settings_scr, on_swipe, LV_EVENT_GESTURE, NULL);

    ui_queue = xQueueCreate(5, sizeof(ui_msg_t));
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
}

void ui_post_notification_msg(const char *text)
{
    ui_msg_t msg = { .type = UI_MSG_NOTIFICATION };
    strncpy(msg.data.notification.notification, text, sizeof(msg.data.notification.notification) - 1);
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_clock_msg(int hour, int min, int dow, int day, int month, bool valid)
{
    ui_msg_t msg = { .type = UI_MSG_CLOCK, .data.clock = { hour, min, dow, day, month, valid } };
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_battery_msg(int bat_pct, bool is_charging)
{
    ui_msg_t msg = { .type = UI_MSG_BATTERY, .data.battery = { bat_pct, is_charging } };
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_inference_msg(const char *label, float confidence)
{
    ui_msg_t msg = { .type = UI_MSG_INFERENCE };
    strncpy(msg.data.inference.label, label, sizeof(msg.data.inference.label) - 1);
    msg.data.inference.confidence = confidence;
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_audio_level_msg(int level_pct)
{
    ui_msg_t msg = { .type = UI_MSG_AUDIO_LEVEL, .data.audio_level = { level_pct } };
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_alert_icon_msg(const char *label, int level)
{
    ui_msg_t msg = { .type = UI_MSG_ALERT_ICON };
    strncpy(msg.data.alert_icon.label, label, sizeof(msg.data.alert_icon.label) - 1);
    msg.data.alert_icon.level = level;
    xQueueSend(ui_queue, &msg, 0);
}
