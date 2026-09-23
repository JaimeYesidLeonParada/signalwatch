#include "rtc_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui.h"
#include <stdio.h>

#define PCF85063_ADDR 0x51 
#define CLOCK_TASK_PRIORITY   1
#define CLOCK_UPDATE_MS       (1 * 60 * 1000)

static i2c_master_dev_handle_t rtc_dev_handle;

static uint8_t bcd2dec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }
static uint8_t dec2bcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

static void clock_task(void *arg)
{
    char time_str[16];
    while (1) {
        rtc_get_time_str(time_str, sizeof(time_str));
        ui_post_clock_msg(time_str);
        vTaskDelay(pdMS_TO_TICKS(CLOCK_UPDATE_MS));
    }
}

void rtc_task_start(void)
{
    xTaskCreate(clock_task, "clock_task", 4096, NULL, CLOCK_TASK_PRIORITY, NULL);
}

void rtc_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF85063_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &rtc_dev_handle));
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
    }
}

void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hr, uint8_t min, uint8_t sec)
{
    // El PCF85063 guarda el año de 0 a 99
    if (year >= 2000) year -= 2000;

    // Buffer: [Registro inicial] + [Seg] + [Min] + [Hora] + [Día] + [DíaSemana] + [Mes] + [Año]
    uint8_t write_data[8];
    write_data[0] = 0x04; // Registro 0x04 (Segundos)
    
    write_data[1] = dec2bcd(sec);
    write_data[2] = dec2bcd(min);
    write_data[3] = dec2bcd(hr);
    write_data[4] = dec2bcd(day);
    write_data[5] = 0; // Día de la semana (se puede dejar en 0 por ahora)
    write_data[6] = dec2bcd(month);
    write_data[7] = dec2bcd((uint8_t)year);

    esp_err_t err = i2c_master_transmit(rtc_dev_handle, write_data, sizeof(write_data), -1);
}