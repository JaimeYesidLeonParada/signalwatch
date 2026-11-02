#include "ui_vu.h"

static lv_obj_t * vu_icon = NULL;

lv_obj_t *ui_vu_create(lv_obj_t *scr)
{
    vu_icon = lv_label_create(scr);
    lv_obj_set_style_text_font(vu_icon, &lv_font_montserrat_38, LV_PART_MAIN);
    lv_obj_set_style_text_color(vu_icon, lv_color_hex(0x00FFBB), LV_PART_MAIN);
    lv_obj_align(vu_icon, LV_ALIGN_BOTTOM_LEFT, 10, -20);
    return vu_icon;
}

void ui_vu_update(int level)
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
