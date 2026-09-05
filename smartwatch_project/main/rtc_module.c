#include "rtc_module.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "rtc_module";
#define PCF85063_ADDR 0x51 // Dirección estándar I2C de este chip

static i2c_master_dev_handle_t rtc_dev_handle;

// Funciones para convertir BCD a Decimal
static uint8_t bcd2dec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

void rtc_init(i2c_master_bus_handle_t bus_handle)
{
    // Agregamos el dispositivo RTC al bus I2C existente (Nuevo estándar v6.0)
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF85063_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &rtc_dev_handle));
    ESP_LOGI(TAG, "RTC PCF85063 device added to I2C bus.");
}

void rtc_get_time_str(char *time_str, size_t max_len)
{
    uint8_t rx_data[7];
    uint8_t reg = 0x04; // Registro 0x04: Empiezan los segundos

    // Le pedimos 7 bytes al chip a partir del registro 0x04
    esp_err_t err = i2c_master_transmit_receive(rtc_dev_handle, &reg, 1, rx_data, sizeof(rx_data), -1);
    
    if (err == ESP_OK) {
        // Limpiamos los bits de estado y convertimos
        uint8_t sec = bcd2dec(rx_data[0] & 0x7F);
        uint8_t min = bcd2dec(rx_data[1] & 0x7F);
        uint8_t hr  = bcd2dec(rx_data[2] & 0x3F);
        
        snprintf(time_str, max_len, "%02d:%02d:%02d", hr, min, sec);
    } else {
        snprintf(time_str, max_len, "Error RTC");
        ESP_LOGE(TAG, "Failed to read RTC");
    }
}