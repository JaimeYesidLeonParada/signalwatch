#include "ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

#define LCD_MOSI_PIN    (GPIO_NUM_2)  
#define LCD_SCLK_PIN    (GPIO_NUM_1)  
#define LCD_CS_PIN      (GPIO_NUM_5)
#define LCD_DC_PIN      (GPIO_NUM_3)
#define LCD_RST_PIN     (GPIO_NUM_4)
#define LCD_BL_PIN      (GPIO_NUM_6)

#define LCD_HOST        SPI2_HOST
#define LCD_PIXEL_CLK   (40 * 1000 * 1000)
#define LCD_H_RES       240
#define LCD_V_RES       280

static lv_obj_t * clock_label = NULL;
static lv_obj_t * battery_label = NULL;
static lv_obj_t * usb_label = NULL;
static lv_obj_t * inference_label = NULL;
static lv_obj_t * vu_label = NULL;
static lv_obj_t * vu_bar = NULL;

static const char *TAG = "esp32_c6_ui_module";
lv_disp_t * disp;

typedef enum {
    UI_MSG_CLOCK,
    UI_MSG_BATTERY,
    UI_MSG_INFERENCE,
    UI_MSG_AUDIO_LEVEL,
} ui_msg_type_t;

typedef struct {
    ui_msg_type_t type;
    union {
        struct { char time_str[16]; } clock;
        struct { int bat_pct; bool is_charging; } battery;
        struct { char label[32]; float confidence; } inference;
        struct { int level; } audio_level;
    } data;
} ui_msg_t;

static QueueHandle_t ui_queue = NULL;
static void ui_task(void *arg);

extern const lv_font_t nine_pin_70;

void ui_display_init(void)
{
    gpio_reset_pin(LCD_BL_PIN);
    gpio_set_direction(LCD_BL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_BL_PIN, 1);

    // 3. Configure SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_SCLK_PIN,
        .mosi_io_num = LCD_MOSI_PIN,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t)
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 4. Configure LCD IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC_PIN,
        .cs_gpio_num = LCD_CS_PIN,
        .pclk_hz = LCD_PIXEL_CLK,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    // 5. Initialize ST7789 panel
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 20)); 
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true)); 
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // 6. Initialize LVGL
    ESP_LOGI(TAG, "Initialize LVGL graphics library");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    // 7. Add LCD screen to LVGL
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * LCD_V_RES / 10, // 10% of screen size for DMA buffer
        .double_buffer = 1,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .flags = {
            .buff_dma = true,
        }
    };
    disp = lvgl_port_add_disp(&disp_cfg);

    lv_obj_set_style_bg_color(lv_disp_get_scr_act(disp), lv_color_hex(0x000000), LV_PART_MAIN);

    ui_queue = xQueueCreate(5, sizeof(ui_msg_t));
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
}

static void ui_update_clock(const char *time_str)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    if (clock_label == NULL) {
        clock_label = lv_label_create(scr);
        lv_obj_set_style_text_color(clock_label, lv_color_hex(0x00FF00), LV_PART_MAIN);
        lv_obj_set_style_text_font(clock_label, &nine_pin_70, LV_PART_MAIN);
        lv_obj_align(clock_label, LV_ALIGN_TOP_MID, 0, 50);
    }

    int hr = 0, min = 0;
    sscanf(time_str, "%d:%d", &hr, &min);

    int hr12 = hr % 12;
    if (hr12 == 0) hr12 = 12;

    lv_label_set_text_fmt(clock_label, "%d:%02d", hr12, min);
}

void ui_update_battery(int bat_pct, bool is_charging)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);

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

static void ui_update_inference(const char *label, float confidence)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    if (inference_label == NULL) {
        inference_label = lv_label_create(scr);
        lv_obj_set_style_text_color(inference_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_align(inference_label, LV_ALIGN_TOP_MID, 0, 125);
    }
    int pct = (int)(confidence * 100.0f + 0.5f); // redondeo
    lv_label_set_text_fmt(inference_label, "%s: %d%%", label, pct);
}

static void ui_update_vu(int level)
{
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    if (vu_bar == NULL) {
        vu_label = lv_label_create(scr);
        lv_obj_set_style_text_color(vu_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_label_set_text(vu_label, "Volumen");
        lv_obj_align(vu_label, LV_ALIGN_BOTTOM_MID, 0, -50);

        vu_bar = lv_bar_create(scr);
        lv_obj_set_size(vu_bar, 180, 20);
        lv_bar_set_range(vu_bar, 0, 100);
        lv_obj_set_style_bg_color(vu_bar, lv_color_hex(0x00AAFF), LV_PART_INDICATOR);
        lv_obj_align(vu_bar, LV_ALIGN_BOTTOM_MID, 0, -20);
    }
    lv_bar_set_value(vu_bar, level, LV_ANIM_OFF); // sin animacion, salto instantaneo
}

static void ui_task(void *arg)
{
    ui_msg_t msg;
    while (1) {
        if (xQueueReceive(ui_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (lvgl_port_lock(-1)) {
                switch (msg.type) {
                    case UI_MSG_CLOCK:
                    ui_update_clock(msg.data.clock.time_str);
                    break;
                    
                    case UI_MSG_BATTERY:
                    ui_update_battery(msg.data.battery.bat_pct, msg.data.battery.is_charging);
                    break;
    
                    case UI_MSG_INFERENCE:
                    ui_update_inference(msg.data.inference.label, msg.data.inference.confidence);
                    break;

                    case UI_MSG_AUDIO_LEVEL:
                    ui_update_vu(msg.data.audio_level.level);
                    break;
                }
                lvgl_port_unlock();
            }
        }
    }
}

void ui_post_clock_msg(const char *time_str)
{
    ui_msg_t msg = { .type = UI_MSG_CLOCK };
    strncpy(msg.data.clock.time_str, time_str, sizeof(msg.data.clock.time_str) - 1);
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