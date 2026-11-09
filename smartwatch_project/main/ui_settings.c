#include "ui_settings.h"
#include <string.h>

extern const lv_font_t nine_pin_70;

#define COLOR_TEXT      0xFFFFFF
#define COLOR_SELECTED  0xFFB000   // borde de la zona seleccionada
#define COLOR_BUTTON    0x303030
#define COLOR_PRESSED   0x606060

static settings_input_cb_t input_cb;

static lv_obj_t *hour_box, *minute_box;
static lv_obj_t *hour_label, *minute_label;
static lv_obj_t *ampm_label, *date_label;

static void emit(settings_input_t input)
{
    if (input_cb) input_cb(input);
}

static void on_field_clicked(lv_event_t *e)
{
    emit((settings_input_t)(intptr_t)lv_event_get_user_data(e));
}

// Un toque suma o resta uno; mantenerlo pulsado repite (LONG_PRESSED_REPEAT). Se usa
// SHORT_CLICKED y no CLICKED porque este ultimo tambien salta al soltar tras una pulsacion
// larga, y el valor cambiaria una vez de mas.
static void on_step_event(lv_event_t *e)
{
    emit((settings_input_t)(intptr_t)lv_event_get_user_data(e));
}

static lv_obj_t *new_label(lv_obj_t *parent, const lv_font_t *font)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    if (font) lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    return label;
}

// Rectangulo sin adornos del tema (borde, relleno, scroll) para usar como zona tocable.
static lv_obj_t *new_zone(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *zone = lv_obj_create(parent);
    lv_obj_set_pos(zone, x, y);
    lv_obj_set_size(zone, w, h);
    lv_obj_remove_flag(zone, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(zone, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(zone, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(zone, 12, LV_PART_MAIN);
    return zone;
}

// Zona de hora o minutos: con la fuente grande dentro.
static lv_obj_t *new_field(lv_obj_t *scr, int x, settings_input_t input, lv_obj_t **label)
{
    lv_obj_t *zone = new_zone(scr, x, 28, 95, 90);
    lv_obj_add_event_cb(zone, on_field_clicked, LV_EVENT_CLICKED, (void *)(intptr_t)input);
    *label = new_label(zone, &nine_pin_70);
    lv_obj_remove_flag(*label, LV_OBJ_FLAG_CLICKABLE);   // el toque lo recibe la zona
    lv_obj_center(*label);
    return zone;
}

// Boton grande con "+" o "-" dibujado con barras, para no depender de la fuente.
static void new_step_button(lv_obj_t *scr, int x, bool plus, settings_input_t input)
{
    lv_obj_t *btn = new_zone(scr, x, 198, 105, 74);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BUTTON), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRESSED), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 16, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, on_step_event, LV_EVENT_SHORT_CLICKED, (void *)(intptr_t)input);
    lv_obj_add_event_cb(btn, on_step_event, LV_EVENT_LONG_PRESSED_REPEAT, (void *)(intptr_t)input);

    for (int i = 0; i < (plus ? 2 : 1); i++) {
        lv_obj_t *bar = lv_obj_create(btn);
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);   // el toque lo recibe el boton
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(bar, i == 0 ? 44 : 8, i == 0 ? 8 : 44);
        lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
        lv_obj_center(bar);
    }
}

lv_obj_t *ui_settings_create(settings_input_cb_t on_input)
{
    input_cb = on_input;

    lv_obj_t *scr = lv_obj_create(NULL);   // NULL = pantalla nueva, no un hijo de otra
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = new_label(scr, NULL);
    lv_label_set_text(title, "AJUSTES");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    // Hora y minutos son dos zonas tocables; los ':' van entre ellas (la fuente solo tiene
    // digitos, ':', A, P y M).
    hour_box   = new_field(scr, 5,   SETTINGS_INPUT_SELECT_HOUR,   &hour_label);
    minute_box = new_field(scr, 140, SETTINGS_INPUT_SELECT_MINUTE, &minute_label);
    lv_obj_t *colon = new_label(scr, &nine_pin_70);
    lv_label_set_text(colon, ":");
    lv_obj_remove_flag(colon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(colon, LV_ALIGN_TOP_MID, 0, 28 + 10);

    ampm_label = new_label(scr, NULL);
    date_label = new_label(scr, NULL);
    lv_obj_align(ampm_label, LV_ALIGN_TOP_MID, 0, 122);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 146);

    lv_obj_t *hint = new_label(scr, NULL);
    lv_label_set_text(hint, "Desliza para volver");
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 172);

    new_step_button(scr, 10,  false, SETTINGS_INPUT_DECREMENT);
    new_step_button(scr, 125, true,  SETTINGS_INPUT_INCREMENT);

    return scr;
}

static void set_selected(lv_obj_t *zone, bool selected)
{
    lv_obj_set_style_border_width(zone, selected ? 4 : 0, LV_PART_MAIN);
    lv_obj_set_style_border_color(zone, lv_color_hex(COLOR_SELECTED), LV_PART_MAIN);
}

void ui_settings_show_time(const char *time, const char *ampm, const char *date,
                           settings_field_t selected)
{
    // "H:MM" -> hora y minutos, cada uno en su zona.
    char hour[4] = "", minute[4] = "";
    const char *colon = strchr(time, ':');
    if (colon != NULL) {
        size_t hour_len = (size_t)(colon - time);
        if (hour_len >= sizeof(hour)) hour_len = sizeof(hour) - 1;
        memcpy(hour, time, hour_len);
        hour[hour_len] = '\0';
        strncpy(minute, colon + 1, sizeof(minute) - 1);
    }

    lv_label_set_text(hour_label, hour);
    lv_label_set_text(minute_label, minute);
    lv_obj_center(hour_label);     // el ancho cambia con 1 o 2 digitos
    lv_obj_center(minute_label);
    lv_label_set_text(ampm_label, ampm);
    lv_label_set_text(date_label, date);

    set_selected(hour_box,   selected == SETTINGS_FIELD_HOUR);
    set_selected(minute_box, selected == SETTINGS_FIELD_MINUTE);
}
