#include "ui_clock.h"
#include "clock_format.h"

extern const lv_font_t nine_pin_70;

static lv_obj_t * clock_label = NULL;
static lv_obj_t * date_label = NULL;

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

    char time_text[CLOCK_TIME_LEN], date_text[CLOCK_DATE_LEN];
    clock_format(hr, min, dow, day, month, valid, time_text, sizeof(time_text), date_text, sizeof(date_text));
    lv_label_set_text(clock_label, time_text);
    lv_label_set_text(date_label, date_text);
}
