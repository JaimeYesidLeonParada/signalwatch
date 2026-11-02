#include "ui_battery.h"

static lv_obj_t * battery_label = NULL;
static lv_obj_t * usb_label = NULL;

void ui_battery_update(lv_obj_t *scr, int bat_pct, bool is_charging)
{
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
