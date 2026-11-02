#include "touch_module.h"
#include "esp_log.h"
#include "lvgl.h"

#define CST816_ADDR 0x15 // Dirección estándar I2C de la familia CST816

static const char *TAG = "touch_module";
static i2c_master_dev_handle_t touch_dev_handle;

// Callback actualizado para LVGL v9
static void touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    uint8_t reg = 0x02; // Registro 0x02 indica el número de dedos (Finger count)
    uint8_t rx_data[5] = {0};

    // Leemos 5 bytes (Dedos, X_High, X_Low, Y_High, Y_Low)
    esp_err_t err = i2c_master_transmit_receive(touch_dev_handle, &reg, 1, rx_data, sizeof(rx_data), -1);

    if (err == ESP_OK) {
        uint8_t fingers = rx_data[0] & 0x0F;

        if (fingers > 0) {
            // Unimos los bytes altos y bajos de las coordenadas
            uint16_t x = ((rx_data[1] & 0x0F) << 8) | rx_data[2];
            uint16_t y = ((rx_data[3] & 0x0F) << 8) | rx_data[4];

            data->point.x = x;
            data->point.y = y;
            data->state = LV_INDEV_STATE_PRESSED; // v9: PRESSED en lugar de PR
        } else {
            data->state = LV_INDEV_STATE_RELEASED; // v9: RELEASED en lugar de REL
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void touch_init(i2c_master_bus_handle_t bus_handle, lv_disp_t *disp)
{
    ESP_LOGI(TAG, "Agregando Touch CST816 al bus I2C...");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CST816_ADDR,
        .scl_speed_hz = 400000, 
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &touch_dev_handle));

    // Sintaxis moderna de LVGL v9 para registrar el touch
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);
    lv_indev_set_display(indev, disp);
    
    ESP_LOGI(TAG, "Driver Touch registrado en LVGL v9 con éxito.");
}