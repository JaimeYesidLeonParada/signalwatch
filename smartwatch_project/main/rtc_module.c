#include "rtc_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "rtc";

#define PCF85063_ADDR 0x51
#define CLOCK_TASK_PRIORITY   1
#define CLOCK_UPDATE_MS       (1 * 60 * 1000)

static i2c_master_dev_handle_t rtc_dev_handle;

static rtc_clock_cb_t clock_cb;

static void clock_task(void *arg)
{
    while (1) {
        rtc_time_t t;
        bool ok = rtc_get_time(&t);
        if (clock_cb) {
            if (ok) clock_cb(t.hr, t.min, t.dow, t.day, t.month, true);
            else    clock_cb(0, 0, 0, 0, 0, false);
        }
        vTaskDelay(pdMS_TO_TICKS(CLOCK_UPDATE_MS));
    }
}

void rtc_task_start(rtc_clock_cb_t on_clock)
{
    clock_cb = on_clock;
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

bool rtc_get_time(rtc_time_t *out)
{
    uint8_t rx_data[RTC_RAW_LEN];
    uint8_t reg = 0x04; // Registro 0x04: Empiezan los segundos

    // Le pedimos 7 bytes al chip a partir del registro 0x04
    esp_err_t err = i2c_master_transmit_receive(rtc_dev_handle, &reg, 1, rx_data, sizeof(rx_data), -1);
    if (err != ESP_OK) return false;

    return rtc_decode_registers(rx_data, out);
}

bool rtc_read_wall_time(wall_time_t *out)
{
    rtc_time_t t;
    if (!rtc_get_time(&t)) return false;
    rtc_time_to_wall_time(&t, out);
    return true;
}

bool rtc_set_time(const wall_time_t *t)
{
    // Buffer: [Registro inicial] + [Seg] + [Min] + [Hora] + [Dia] + [DiaSemana] + [Mes] + [Anio]
    uint8_t write_data[1 + RTC_RAW_LEN];
    write_data[0] = 0x04; // Registro 0x04 (Segundos)

    if (!rtc_encode_registers(t, &write_data[1])) {
        ESP_LOGW(TAG, "hora fuera de rango, no se escribe");
        return false;
    }

    esp_err_t err = i2c_master_transmit(rtc_dev_handle, write_data, sizeof(write_data), -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "fallo al escribir la hora: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}