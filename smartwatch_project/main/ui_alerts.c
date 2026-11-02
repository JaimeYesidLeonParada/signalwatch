#include "ui_alerts.h"
#include "alert_catalog.h"
#include "alert_module.h"
#include "ui_notification.h"

extern const lv_font_t fa_icons_32;

// Un icono y un ultimo nivel por cada clase del catalogo, con el mismo indice.
static lv_obj_t * alert_icons[ALERT_CATALOG_MAX];
static alert_level_t alert_prev_level[ALERT_CATALOG_MAX];

static void update_alert_text(lv_obj_t *scr, size_t idx, alert_level_t level)
{
    if (level == alert_prev_level[idx]) return;   // el nivel no cambio
    alert_prev_level[idx] = level;
    if (level >= ALERT_LOW && level <= ALERT_HIGH) {
        ui_notification_show_text(scr, alert_catalog_get(idx)->msg[level - ALERT_LOW]);
    } else {
        ui_notification_show_text(scr, "");
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

void ui_alerts_create(lv_obj_t *scr, lv_obj_t *anchor)
{
    // Un icono por clase del catalogo, en fila: el primero junto al ancla y cada uno
    // a continuacion del anterior.
    for (size_t i = 0; i < alert_catalog_count(); i++) {
        lv_obj_t *icon = lv_label_create(scr);
        lv_obj_set_style_text_font(icon, &fa_icons_32, LV_PART_MAIN);
        lv_obj_set_style_text_color(icon, lv_color_hex(0x660066), LV_PART_MAIN);
        lv_obj_set_style_text_opa(icon, LV_OPA_10, LV_PART_MAIN);
        lv_label_set_text(icon, alert_catalog_get(i)->glyph);
        if (i == 0) lv_obj_align_to(icon, anchor, LV_ALIGN_OUT_RIGHT_MID, -20, 0);
        else        lv_obj_align_to(icon, alert_icons[i - 1], LV_ALIGN_OUT_RIGHT_MID, 35, 0);
        alert_icons[i] = icon;
    }
}

void ui_alerts_update(lv_obj_t *scr, const char *label, int level)
{
    int idx = alert_catalog_index_of(label);
    if (idx >= 0) {
       set_alert_icon_style(alert_icons[idx], (alert_level_t)level);
       update_alert_text(scr, (size_t)idx, (alert_level_t)level);
    }
}
