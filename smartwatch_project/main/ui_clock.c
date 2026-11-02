#include "ui_clock.h"

extern const lv_font_t nine_pin_70;

static lv_obj_t * clock_label = NULL;
static lv_obj_t * date_label = NULL;

static const char *const DAY_NAMES[]   = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
static const char *const MONTH_NAMES[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                                          "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};

void ui_clock_update(lv_obj_t *scr, int hr, int min, int dow, int day, int month, bool valid)
{
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
