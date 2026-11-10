#include "ui.h"
#include "ui_clock.h"
#include "ui_battery.h"
#include "ui_notification.h"
#include "ui_vu.h"
#include "ui_alerts.h"
#include "ui_settings.h"
#include "ui_sleep.h"
#include "alert_urgent.h"
#include "clock_format.h"
#include "time_edit.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>
#include "esp_lvgl_port.h"
#include "esp_log.h"

static const char *TAG = "ui";

static lv_disp_t * disp;
// Dos pantallas: la principal (reloj y alertas) y la de ajustes. Los mensajes siempre se
// dibujan en main_scr, este o no visible, para que no aparezcan widgets en la de ajustes.
static lv_obj_t * main_scr;
static lv_obj_t * settings_scr;
// De donde sale la hora que muestran los ajustes; la conecta app_init.c.
static time_source_t time_source;
// A donde se guarda la hora editada; tambien la conecta app_init.c.
static time_sink_t time_sink;

typedef enum {
    UI_MSG_CLOCK,
    UI_MSG_BATTERY,
    UI_MSG_INFERENCE,
    UI_MSG_AUDIO_LEVEL,
    UI_MSG_ALERT_ICON,
    UI_MSG_NOTIFICATION,
    UI_MSG_WAKE,
    UI_MSG_BUTTON,
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
static void (*button_feedback)(void);

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
        // Una alerta muy clara saca al usuario de los ajustes; el resto espera a que el
        // usuario vuelva (la vibracion avisa igual).
        if (alert_is_urgent(msg->data.inference.label, msg->data.inference.confidence) &&
            lv_screen_active() == settings_scr) {
            lv_screen_load(main_scr);
        }
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

        case UI_MSG_WAKE:
        ui_sleep_activity();
        break;

        case UI_MSG_BUTTON:
        // Con la pantalla apagada el boton solo la enciende; con ella encendida da feedback.
        if (!ui_sleep_activity() && button_feedback != NULL) {
            button_feedback();
        }
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

// Hora y fecha que el usuario esta editando en los ajustes. Todo corre en el hilo de LVGL,
// con el lock tomado.
static time_edit_t edit;

// Dibuja en los ajustes el estado actual de la edicion.
static void render_settings(void)
{
    wall_time_t t;
    time_edit_to_wall_time(&edit, &t);
    clock_texts_t texts;
    clock_format_full(&t, true, &texts);

    settings_view_t view = {
        .time = texts.time, .ampm = texts.ampm,
        .day = texts.day, .month = texts.month, .year = texts.year,
    };
    switch (edit.field) {
        case TIME_FIELD_HOUR:   view.selected = SETTINGS_FIELD_HOUR;   break;
        case TIME_FIELD_MINUTE: view.selected = SETTINGS_FIELD_MINUTE; break;
        case TIME_FIELD_DAY:    view.selected = SETTINGS_FIELD_DAY;    break;
        case TIME_FIELD_MONTH:  view.selected = SETTINGS_FIELD_MONTH;  break;
        default:                view.selected = SETTINGS_FIELD_YEAR;   break;
    }
    ui_settings_show(&view);
}

// Guarda la hora editada. Si se guardo, el reloj principal se actualiza ya (sin esperar al
// ciclo de cada minuto) y la pantalla vuelve a la principal; si no, se queda en los ajustes
// con un aviso para que el usuario pueda reintentar.
static void save_edited_time(void)
{
    wall_time_t t;
    time_edit_to_wall_time(&edit, &t);

    if (time_sink == NULL || !time_sink(&t)) {
        ui_settings_show_message("No se pudo guardar");
        return;
    }
    ui_clock_update(main_scr, t.hour, t.minute, t.dow, t.day, t.month, true);
    lv_screen_load_anim(main_scr, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
}

// Traduce lo que hace el usuario en la pantalla a operaciones de time_edit.
static void on_settings_input(settings_input_t input)
{
    switch (input) {
        case SETTINGS_INPUT_SELECT_HOUR:   edit.field = TIME_FIELD_HOUR;   break;
        case SETTINGS_INPUT_SELECT_MINUTE: edit.field = TIME_FIELD_MINUTE; break;
        case SETTINGS_INPUT_SELECT_DAY:    edit.field = TIME_FIELD_DAY;    break;
        case SETTINGS_INPUT_SELECT_MONTH:  edit.field = TIME_FIELD_MONTH;  break;
        case SETTINGS_INPUT_SELECT_YEAR:   edit.field = TIME_FIELD_YEAR;   break;
        case SETTINGS_INPUT_INCREMENT:     time_edit_increment(&edit);     break;
        case SETTINGS_INPUT_DECREMENT:     time_edit_decrement(&edit);     break;
        case SETTINGS_INPUT_SAVE:          save_edited_time();             return;
    }
    render_settings();
}

// Al empezar a mostrarse los ajustes se lee la hora actual del RTC y se empieza a editar
// desde ella (o desde la hora por defecto si no hay una valida). Lo que se edito antes y no
// se guardo se descarta.
static void on_settings_open(lv_event_t *e)
{
    (void)e;
    wall_time_t now;
    if (time_source != NULL && time_source(&now)) {
        time_edit_init(&edit, now.year, now.month, now.day, now.hour, now.minute);
    } else {
        time_edit_init(&edit, TIME_EDIT_DEFAULT_YEAR, TIME_EDIT_DEFAULT_MONTH,
                       TIME_EDIT_DEFAULT_DAY, TIME_EDIT_DEFAULT_HOUR, TIME_EDIT_DEFAULT_MINUTE);
    }
    render_settings();
}

void ui_set_time_source(time_source_t source)
{
    time_source = source;
}

void ui_set_time_sink(time_sink_t sink)
{
    time_sink = sink;
}

void ui_set_button_feedback(void (*feedback)(void))
{
    button_feedback = feedback;
}

void ui_set_display_power(display_power_t power)
{
    ui_sleep_set_power(power);
}

// La pantalla se acaba de apagar por inactividad. Si estaba en los ajustes vuelve a la
// principal (lo editado sin guardar se descarta), para que al despertar se vea el reloj.
static void on_screen_sleep(void)
{
    if (lv_screen_active() == settings_scr) {
        lv_screen_load(main_scr);
    }
}

void ui_start(lv_disp_t *display)
{
    disp = display;

    // La tarea de LVGL ya esta dibujando: crear widgets sin su lock corrompe su memoria y el
    // hilo se queda colgado (watchdog). Todo lo que toca LVGL va entre lock y unlock.
    lvgl_port_lock(-1);
    main_scr = lv_disp_get_scr_act(disp);
    lv_obj_t * scr = main_scr;
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    ui_alerts_create(scr, ui_vu_create(scr));

    // Sin scroll en las pantallas: si no, el dedo las desplaza en vez de dar un gesto.
    lv_obj_remove_flag(main_scr, LV_OBJ_FLAG_SCROLLABLE);
    settings_scr = ui_settings_create(on_settings_input);
    lv_obj_add_event_cb(main_scr, on_swipe, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(settings_scr, on_swipe, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(settings_scr, on_settings_open, LV_EVENT_SCREEN_LOAD_START, NULL);
    on_settings_open(NULL);   // deja la pantalla con texto desde el inicio
    ui_sleep_create(disp, on_screen_sleep);

    lv_mem_monitor_t mem;
    lv_mem_monitor(&mem);
    ESP_LOGI(TAG, "memoria de LVGL: %u%% usada (%u de %u bytes), pico %u bytes, fragmentacion %u%%",
             (unsigned)mem.used_pct, (unsigned)(mem.total_size - mem.free_size), (unsigned)mem.total_size, (unsigned)mem.max_used,
             (unsigned)mem.frag_pct);
    lvgl_port_unlock();

    ui_queue = xQueueCreate(5, sizeof(ui_msg_t));
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
}

void ui_post_notification_msg(const char *text)
{
    ui_msg_t msg = { .type = UI_MSG_NOTIFICATION };
    strncpy(msg.data.notification.notification, text, sizeof(msg.data.notification.notification) - 1);
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_wake_msg(void)
{
    ui_msg_t msg = { .type = UI_MSG_WAKE };
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_button_msg(void)
{
    ui_msg_t msg = { .type = UI_MSG_BUTTON };
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
