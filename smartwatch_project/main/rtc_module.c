#include "rtc_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PCF85063_ADDR 0x51
#define CLOCK_TASK_PRIORITY   1
#define CLOCK_UPDATE_MS       (1 * 60 * 1000)

static i2c_master_dev_handle_t rtc_dev_handle;

static uint8_t bcd2dec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }
static uint8_t dec2bcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

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
    uint8_t rx_data[7];
    uint8_t reg = 0x04; // Registro 0x04: Empiezan los segundos

    // Le pedimos 7 bytes al chip a partir del registro 0x04
    esp_err_t err = i2c_master_transmit_receive(rtc_dev_handle, &reg, 1, rx_data, sizeof(rx_data), -1);
    if (err != ESP_OK) return false;

    out->sec   = bcd2dec(rx_data[0] & 0x7F);
    out->min   = bcd2dec(rx_data[1] & 0x7F);
    out->hr    = bcd2dec(rx_data[2] & 0x3F);
    out->day   = bcd2dec(rx_data[3] & 0x3F);
    out->dow   = bcd2dec(rx_data[4] & 0x3F);
    out->month = bcd2dec(rx_data[5] & 0x1F);
    out->year  = bcd2dec(rx_data[6]);

    // Con el chip sin configurar o sin pila los campos pueden salir fuera de rango;
    // se descartan para que nadie indexe tablas con ellos.
    bool plausible = out->hr <= 23 && out->min <= 59 && out->dow <= 6 &&
                     out->day >= 1 && out->day <= 31 && out->month >= 1 && out->month <= 12;
    return plausible;
}

void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hr, uint8_t min, uint8_t sec)
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
    write_data[5] = dec2bcd(dow);
    write_data[6] = dec2bcd(month);
    write_data[7] = dec2bcd((uint8_t)year);

    esp_err_t err = i2c_master_transmit(rtc_dev_handle, write_data, sizeof(write_data), -1);
}