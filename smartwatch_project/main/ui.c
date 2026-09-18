#include "ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
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

static lv_obj_t * title_label = NULL;
static lv_obj_t * subtitle_label = NULL;
static lv_obj_t * btn = NULL;
static const char *TAG = "esp32_c6_ui_module";
lv_disp_t * disp;

typedef enum {
    UI_MSG_CLOCK,
    UI_MSG_BATTERY,
    UI_MSG_INFERENCE,
} ui_msg_type_t;

typedef struct {
    ui_msg_type_t type;
    union {
        struct { char time_str[16]; } clock;
        struct { int bat_mv; int bat_pct; } battery;
        struct { char label[32]; float confidence; } inference;
    } data;
} ui_msg_t;

static QueueHandle_t ui_queue = NULL;
static void ui_task(void *arg);

// Ultimo valor conocido de cada dato, para recomponer la linea de telemetria
static char s_time_str[16] = "--:--:--";
static int  s_bat_mv = 0;
static int  s_bat_pct = 0;

void ui_display_init(void)
{
     // 2. Turn on LCD backlight
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

    ui_queue = xQueueCreate(5, sizeof(ui_msg_t));
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
}

// Se ejecuta automáticamente al tocar el botón en pantalla
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;
        // Obtenemos el texto de adentro del botón y lo cambiamos
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Tocado: %d", cnt);
    }
}

static void ui_draw_text_internal(const char * title_str, const char * subtitle_str)
{
    // 1. Get the active screen and set background
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1E1E1E), LV_PART_MAIN);

    // 2. Title creation/update
    if (title_label == NULL) {
        // Create it only the first time
        title_label = lv_label_create(scr);
        
        lv_obj_set_style_text_color(title_label, lv_color_hex(0x00FF00), LV_PART_MAIN);
        lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);
    }
    // Set the dynamic text
    lv_label_set_text(title_label, title_str);

    // 3. Subtitle creation/update
    if (subtitle_label == NULL) {
        // Create it only the first time
        subtitle_label = lv_label_create(scr);
        lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
    }
    // Set the dynamic text
    lv_label_set_text(subtitle_label, subtitle_str);
    lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(subtitle_label, title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    // 4. Botón interactivo para probar el touch
    if (btn == NULL) {
        btn = lv_btn_create(scr);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20); // Abajo en el centro
        
        // Le asignamos el evento de toque
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
        
        // Le ponemos texto al botón
        lv_obj_t * btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, "TOCAME");
        lv_obj_center(btn_label);
    }
}

static void ui_redraw_telemetry(void)
{
    char subtitle[128];
    snprintf(subtitle, sizeof(subtitle),
             "ESP32-C6: Activa\nTiempo: %s\nBateria: %d mV (%d%%)",
             s_time_str, s_bat_mv, s_bat_pct);
    ui_draw_text_internal("EDGE Hardware", subtitle);
}

static void ui_task(void *arg)
{
    ui_msg_t msg;
    while (1) {
        if (xQueueReceive(ui_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (lvgl_port_lock(-1)) {
                switch (msg.type) {
                    case UI_MSG_CLOCK:
                        strncpy(s_time_str, msg.data.clock.time_str, sizeof(s_time_str) - 1);
                        s_time_str[sizeof(s_time_str) - 1] = '\0';
                        ui_redraw_telemetry();
                        break;
                    case UI_MSG_BATTERY:
                        s_bat_mv = msg.data.battery.bat_mv;
                        s_bat_pct = msg.data.battery.bat_pct;
                        ui_redraw_telemetry();
                        break;
                    case UI_MSG_INFERENCE:
                        // Se conecta en el siguiente paso, con el layout nuevo
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

void ui_post_battery_msg(int bat_mv, int bat_pct)
{
    ui_msg_t msg = { .type = UI_MSG_BATTERY, .data.battery = { bat_mv, bat_pct } };
    xQueueSend(ui_queue, &msg, 0);
}

void ui_post_inference_msg(const char *label, float confidence)
{
    ui_msg_t msg = { .type = UI_MSG_INFERENCE };
    strncpy(msg.data.inference.label, label, sizeof(msg.data.inference.label) - 1);
    msg.data.inference.confidence = confidence;
    xQueueSend(ui_queue, &msg, 0);
}