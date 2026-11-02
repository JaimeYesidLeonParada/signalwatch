#include "ui_notification.h"

static lv_obj_t * notification_label = NULL;

void ui_notification_show_inference(lv_obj_t *scr, const char *label, float confidence)
{
     if (notification_label == NULL) {
        notification_label = lv_label_create(scr);
        lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_align(notification_label, LV_ALIGN_CENTER, 0, 150);
    }
    int pct = (int)(confidence * 100.0f + 0.5f); // redondeo
    lv_label_set_text_fmt(notification_label, "%s: %d%%", label, pct);
}

void ui_notification_show_text(lv_obj_t *scr, const char *text)
{
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
