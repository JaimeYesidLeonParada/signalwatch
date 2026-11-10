#include "ui_settings.h"
#include <string.h>

// Fuente de la hora: Nine Pin de 48 px, solo digitos y ':' (nine_pin_70 es la del reloj principal).
extern const lv_font_t nine_pin_48;
// Fuente de la fecha: Montserrat de 28 px (ASCII), generada para el proyecto con lv_font_conv.
extern const lv_font_t date_font_28;

#define COLOR_TEXT      0xFFFFFF
#define COLOR_SELECTED  0xFFB000   // borde de la zona seleccionada
#define COLOR_BUTTON    0x303030
#define COLOR_PRESSED   0x606060

// Distribucion (pantalla de 240x280). La hora y el AM/PM forman una fila; debajo va la fecha
// y abajo los botones.
#define TIME_Y        54
#define TIME_ZONE_W   64
#define TIME_ZONE_H   64
#define DATE_Y        130
#define DATE_ZONE_H   52
#define BUTTONS_Y     210
#define BUTTON_W      80
#define BUTTON_H      48
#define BUTTON_EXT_CLICK 14   // px extra alrededor del boton que tambien responden al toque

static settings_input_cb_t input_cb;

// Una zona tocable y su texto por cada campo editable, con el mismo indice que settings_field_t.
static lv_obj_t *zones[SETTINGS_FIELD_COUNT];
static lv_obj_t *labels[SETTINGS_FIELD_COUNT];
static lv_obj_t *ampm_label;

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

// Zona tocable de un campo editable, con su texto centrado (font NULL = fuente por defecto).
static void new_field(lv_obj_t *scr, settings_field_t field, settings_input_t input,
                      int x, int y, int w, int h, const lv_font_t *font)
{
    lv_obj_t *zone = new_zone(scr, x, y, w, h);
    lv_obj_add_event_cb(zone, on_field_clicked, LV_EVENT_CLICKED, (void *)(intptr_t)input);
    lv_obj_t *label = new_label(zone, font);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);   // el toque lo recibe la zona
    lv_obj_center(label);
    zones[field] = zone;
    labels[field] = label;
}

// Boton con "+" o "-" dibujado con barras, para no depender de la fuente.
static void new_step_button(lv_obj_t *scr, int x, bool plus, settings_input_t input)
{
    lv_obj_t *btn = new_zone(scr, x, BUTTONS_Y, BUTTON_W, BUTTON_H);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BUTTON), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRESSED), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 16, LV_PART_MAIN);
    // El dedo no acierta con precision en un boton tan chico: el area que responde al toque es
    // mayor que lo que se dibuja. Con 32 px entre los dos botones, 14 px extra por lado no se pisan.
    lv_obj_set_ext_click_area(btn, BUTTON_EXT_CLICK);
    lv_obj_add_event_cb(btn, on_step_event, LV_EVENT_SHORT_CLICKED, (void *)(intptr_t)input);
    lv_obj_add_event_cb(btn, on_step_event, LV_EVENT_LONG_PRESSED_REPEAT, (void *)(intptr_t)input);

    for (int i = 0; i < (plus ? 2 : 1); i++) {
        lv_obj_t *bar = lv_obj_create(btn);
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);   // el toque lo recibe el boton
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(bar, i == 0 ? 32 : 6, i == 0 ? 6 : 32);
        lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 3, LV_PART_MAIN);
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
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Fila de la hora: HH : MM AM/PM. Cada numero es una zona tocable; el ':' va entre ellas
    // y el AM/PM a la derecha, a la altura de la base de los digitos.
    const int hour_x = 22, colon_x = hour_x + TIME_ZONE_W;
    const int minute_x = colon_x + 31;   // 31 px = ancho de un digito de la fuente de 48 px
    new_field(scr, SETTINGS_FIELD_HOUR,   SETTINGS_INPUT_SELECT_HOUR,   hour_x,   TIME_Y,
              TIME_ZONE_W, TIME_ZONE_H, &nine_pin_48);
    new_field(scr, SETTINGS_FIELD_MINUTE, SETTINGS_INPUT_SELECT_MINUTE, minute_x, TIME_Y,
              TIME_ZONE_W, TIME_ZONE_H, &nine_pin_48);
    lv_obj_t *colon = new_label(scr, &nine_pin_48);
    lv_label_set_text(colon, ":");
    lv_obj_remove_flag(colon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(colon, colon_x, TIME_Y + (TIME_ZONE_H - 41) / 2);   // 41 px = alto de los digitos

    ampm_label = new_label(scr, NULL);
    lv_obj_align_to(ampm_label, zones[SETTINGS_FIELD_MINUTE], LV_ALIGN_OUT_RIGHT_BOTTOM, 2, -11);

    // Fecha: dia, mes y anio, tres zonas tocables en fila, mas grandes que su texto.
    new_field(scr, SETTINGS_FIELD_DAY,   SETTINGS_INPUT_SELECT_DAY,   8,   DATE_Y, 62, DATE_ZONE_H, &date_font_28);
    new_field(scr, SETTINGS_FIELD_MONTH, SETTINGS_INPUT_SELECT_MONTH, 76,  DATE_Y, 70, DATE_ZONE_H, &date_font_28);
    new_field(scr, SETTINGS_FIELD_YEAR,  SETTINGS_INPUT_SELECT_YEAR,  152, DATE_Y, 82, DATE_ZONE_H, &date_font_28);

    new_step_button(scr, 24,  false, SETTINGS_INPUT_DECREMENT);
    new_step_button(scr, 136, true,  SETTINGS_INPUT_INCREMENT);

    return scr;
}

static void set_selected(lv_obj_t *zone, bool selected)
{
    lv_obj_set_style_border_width(zone, selected ? 4 : 0, LV_PART_MAIN);
    lv_obj_set_style_border_color(zone, lv_color_hex(COLOR_SELECTED), LV_PART_MAIN);
}

static void set_field_text(settings_field_t field, const char *text)
{
    lv_label_set_text(labels[field], text);
    lv_obj_center(labels[field]);   // el ancho cambia con el texto
}

void ui_settings_show(const settings_view_t *view)
{
    // "H:MM" -> hora y minutos, cada uno en su zona.
    char hour[4] = "", minute[4] = "";
    const char *colon = strchr(view->time, ':');
    if (colon != NULL) {
        size_t hour_len = (size_t)(colon - view->time);
        if (hour_len >= sizeof(hour)) hour_len = sizeof(hour) - 1;
        memcpy(hour, view->time, hour_len);
        hour[hour_len] = '\0';
        strncpy(minute, colon + 1, sizeof(minute) - 1);
    }

    set_field_text(SETTINGS_FIELD_HOUR, hour);
    set_field_text(SETTINGS_FIELD_MINUTE, minute);
    set_field_text(SETTINGS_FIELD_DAY, view->day);
    set_field_text(SETTINGS_FIELD_MONTH, view->month);
    set_field_text(SETTINGS_FIELD_YEAR, view->year);
    lv_label_set_text(ampm_label, view->ampm);
    lv_obj_align_to(ampm_label, zones[SETTINGS_FIELD_MINUTE], LV_ALIGN_OUT_RIGHT_BOTTOM, 2, -11);

    for (int f = 0; f < SETTINGS_FIELD_COUNT; f++) {
        set_selected(zones[f], f == (int)view->selected);
    }
}
