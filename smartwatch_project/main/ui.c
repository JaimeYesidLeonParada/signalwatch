#include "ui.h"
#include "alert_catalog.h"
#include "alert_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>
#include "lvgl.h"
#include "esp_lvgl_port.h"

static lv_obj_t * clock_label = NULL;
static lv_obj_t * date_label = NULL;
static lv_obj_t * battery_label = NULL;
static lv_obj_t * usb_label = NULL;
static lv_obj_t * notification_label = NULL;
static lv_obj_t * vu_icon = NULL;
// Un icono y un ultimo nivel por cada clase del catalogo, con el mismo indice.
static lv_obj_t * alert_icons[ALERT_CATALOG_MAX];
static alert_level_t alert_prev_level[ALERT_CATALOG_MAX];

static lv_disp_t * disp;

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
static void ui_task(void *arg);
static void ui_update_notification(const char *text);

extern const lv_font_t nine_pin_70;
extern const lv_font_t fa_icons_32;

static void update_alert_text(size_t idx, alert_level_t level)
{
    if (level == alert_prev_level[idx]) return;   // el nivel no cambio
    alert_prev_level[idx] = level;
    if (level >= ALERT_LOW && level <= ALERT_HIGH) {
        ui_update_notification(alert_catalog_get(idx)->msg[level - ALERT_LOW]);
    } else {
        ui_update_notification("");
    }
}

static void set_alert_icon_style(lv_obj_t *icon, alert_level_t level)
{
    switch (level) {
        case ALERT_HIGH:
            lv_obj_set_style_text_color(icon, lv_color_hex(0xFF0000), LV_PART_MAIN);
            lv_obj_set_style_text_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
            break;
        case ALERT_MEDIUM:
            lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_text_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
            break;
        case ALERT_LOW:
            lv_obj_set_style_text_color(icon, lv_color_hex(0x660066), LV_PART_MAIN);
            lv_obj_set_style_text_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
            break;
        default: // NONE - a la escucha
            lv_obj_set_style_text_color(icon, lv_color_hex(0x660066), LV_PART_MAIN);
            lv_obj_set_style_text_opa(icon, LV_OPA_10, LV_PART_MAIN);
            break;
    }
}

void ui_alert_icon_init(void)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);

    vu_icon = lv_label_create(scr);
    lv_obj_set_style_text_font(vu_icon, &lv_font_montserrat_38, LV_PART_MAIN);
    lv_obj_set_style_text_color(vu_icon, lv_color_hex(0x00FFBB), LV_PART_MAIN);
    lv_obj_align(vu_icon, LV_ALIGN_BOTTOM_LEFT, 10, -20);
    
    // Un icono por clase del catalogo, en fila: el primero junto al VU y cada uno
    // a continuacion del anterior.
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        lv_obj_t *icon = lv_label_create(scr);
        lv_obj_set_style_text_font(icon, &fa_icons_32, LV_PART_MAIN);
        lv_obj_set_style_text_color(icon, lv_color_hex(0x660066), LV_PART_MAIN);
        lv_obj_set_style_text_opa(icon, LV_OPA_10, LV_PART_MAIN);
        lv_label_set_text(icon, alert_catalog_get(i)->glyph);
        if (i == 0) lv_obj_align_to(icon, vu_icon, LV_ALIGN_OUT_RIGHT_MID, -20, 0);
        else        lv_obj_align_to(icon, alert_icons[i - 1], LV_ALIGN_OUT_RIGHT_MID, 35, 0);
        alert_icons[i] = icon;
    }
}

void ui_start(lv_disp_t *display)
{
    disp = display;
    lv_obj_set_style_bg_color(lv_disp_get_scr_act(disp), lv_color_hex(0x000000), LV_PART_MAIN);
    ui_alert_icon_init();

    ui_queue = xQueueCreate(5, sizeof(ui_msg_t));
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
}

static const char *const DAY_NAMES[]   = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
static const char *const MONTH_NAMES[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                                          "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};

static void ui_update_clock(int hr, int min, int dow, int day, int month, bool valid)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    if (clock_label == NULL) {
        clock_label = lv_label_create(scr);
        lv_obj_set_style_text_color(clock_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(clock_label, &nine_pin_70, LV_PART_MAIN);
        lv_obj_align(clock_label, LV_ALIGN_TOP_MID, 0, 60);
    }

    if (date_label == NULL) {
        date_label = lv_label_create(scr);
        lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, 10, 10);
    }

    if (!valid) hr = min = 0;   // sin lectura del RTC: se muestra 12:00 y sin fecha

    int hr12 = hr % 12;
    if (hr12 == 0) hr12 = 12;

    lv_label_set_text_fmt(clock_label, "%d:%02d", hr12, min);
    if (valid) lv_label_set_text_fmt(date_label, "%s %d %s", DAY_NAMES[dow], day, MONTH_NAMES[month - 1]);
    else       lv_label_set_text(date_label, "");
}

static void ui_update_battery(int bat_pct, bool is_charging)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);

    if (battery_label == NULL) {
        battery_label = lv_label_create(scr);
        lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -14, 10);
    }
    if (usb_label == NULL) {
        usb_label = lv_label_create(scr);
        lv_obj_set_style_text_color(usb_label, lv_color_hex(0x00BFFF), LV_PART_MAIN);
        lv_obj_align_to(usb_label, battery_label, LV_ALIGN_OUT_LEFT_MID, 22, 0);
    }

    const char *symbol;
    if (bat_pct >= 80) symbol = LV_SYMBOL_BATTERY_FULL;
    else if (bat_pct >= 60) symbol = LV_SYMBOL_BATTERY_3;
    else if (bat_pct >= 40) symbol = LV_SYMBOL_BATTERY_2;
    else if (bat_pct >= 15) symbol = LV_SYMBOL_BATTERY_1;
    else symbol = LV_SYMBOL_BATTERY_EMPTY;

    lv_color_t color = is_charging ? lv_color_hex(0x00BFFF) : lv_color_hex(0xFFFFFF);
    lv_obj_set_style_text_color(battery_label, color, LV_PART_MAIN);
    lv_label_set_text(battery_label, symbol);

    lv_label_set_text(usb_label, is_charging ? LV_SYMBOL_USB : "");
}

static void ui_update_inference(const char *label, float confidence)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
     if (notification_label == NULL) {
        notification_label = lv_label_create(scr);
        lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_align(notification_label, LV_ALIGN_CENTER, 0, 150);
    }
    int pct = (int)(confidence * 100.0f + 0.5f); // redondeo
    lv_label_set_text_fmt(notification_label, "%s: %d%%", label, pct);
}

static void ui_update_notification(const char *text)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    if (notification_label == NULL) {
        notification_label = lv_label_create(scr);
        lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_label_set_long_mode(notification_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(notification_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_width(notification_label, 240);
        lv_obj_align(notification_label, LV_ALIGN_CENTER, 0, 20);
    }
    lv_label_set_text(notification_label, text);
}

static void ui_update_vu(int level)
{
    const char *symbol;
    if (level >= 60) {
        symbol = LV_SYMBOL_VOLUME_MAX;
    } else if (level >= 15) {
        symbol = LV_SYMBOL_VOLUME_MID;
    } else {
        symbol = LV_SYMBOL_MUTE;
    }
    
    lv_label_set_text(vu_icon, symbol);
}

static void ui_update_alert_icon(const char *label, int level)
{
    int idx = alert_catalog_index_of(label);
    if (idx >= 0) {
       set_alert_icon_style(alert_icons[idx], (alert_level_t)level);
       update_alert_text((size_t)idx, (alert_level_t)level);
    }
}

static void ui_task(void *arg)
{
    ui_msg_t msg;
    while (1) {
        if (xQueueReceive(ui_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (lvgl_port_lock(-1)) {
                switch (msg.type) {
                    case UI_MSG_CLOCK:
                    ui_update_clock(msg.data.clock.hour, msg.data.clock.min, msg.data.clock.dow,
                                    msg.data.clock.day, msg.data.clock.month, msg.data.clock.valid);
                    break;
                    
                    case UI_MSG_BATTERY:
                    ui_update_battery(msg.data.battery.bat_pct, msg.data.battery.is_charging);
                    break;
    
                    case UI_MSG_INFERENCE:
                    ui_update_inference(msg.data.inference.label, msg.data.inference.confidence);
                    break;

                    case UI_MSG_AUDIO_LEVEL:
                    ui_update_vu(msg.data.audio_level.level);
                    break;

                    case UI_MSG_ALERT_ICON:
                    ui_update_alert_icon(msg.data.alert_icon.label, msg.data.alert_icon.level);
                    break;

                    case UI_MSG_NOTIFICATION:
                    ui_update_notification(msg.data.notification.notification);
                    break;
                }
                lvgl_port_unlock();
            }
        }
    }
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