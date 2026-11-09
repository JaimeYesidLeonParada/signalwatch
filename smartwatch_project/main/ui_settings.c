#include "ui_settings.h"

extern const lv_font_t nine_pin_70;

static lv_obj_t *time_label;
static lv_obj_t *ampm_label;
static lv_obj_t *date_label;

lv_obj_t *ui_settings_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);   // NULL = pantalla nueva, no un hijo de otra
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "AJUSTES");
    lv_obj_set_style_text_color(title, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // La hora usa la misma fuente grande del reloj principal (solo tiene digitos, ":", A, P y M).
    time_label = lv_label_create(scr);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_label, &nine_pin_70, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -40);

    ampm_label = lv_label_create(scr);
    lv_obj_set_style_text_color(ampm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align_to(ampm_label, time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    date_label = lv_label_create(scr);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 50);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Desliza para volver");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    return scr;
}

void ui_settings_show_time(const char *time, const char *ampm, const char *date)
{
    lv_label_set_text(time_label, time);
    lv_label_set_text(ampm_label, ampm);
    lv_label_set_text(date_label, date);
    lv_obj_align_to(ampm_label, time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
}
