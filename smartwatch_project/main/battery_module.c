#include "battery_module.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "battery_module";

#define BAT_ADC_PIN 0 // GPIO0 es ADC1_CHANNEL_0 en el ESP32-C6
#define VOLTAGE_DIVIDER_RATIO 2.43f // Multiplicador de hardware de la placa

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle;
static bool cali_enable = false;
static float smoothed_pct = -1.0f; // -1 = todavia sin ninguna lectura

void battery_init(void)
{
    ESP_LOGI(TAG, "Inicializando ADC para la Batería...");

    // 1. Configurar la Unidad ADC1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // 2. Configurar el Canal 0 (GPIO0)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, // Permite medir hasta ~3.3V
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));

    // 3. Configurar la Calibración exacta (Curva de ajuste para ESP32-C6)
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_0,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    if (ret == ESP_OK) {
        cali_enable = true;
        ESP_LOGI(TAG, "Calibración del ADC exitosa.");
    } else {
        ESP_LOGW(TAG, "Calibración del ADC fallida. Usando valores brutos.");
    }
}

int battery_get_voltage_mv(void)
{
    int raw_val = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_val));

    int voltage_mv = 0;
    if (cali_enable) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw_val, &voltage_mv));
    } else {
        voltage_mv = (raw_val * 3300) / 4095;
    }

    int battery_mv = (int)(voltage_mv * VOLTAGE_DIVIDER_RATIO + 0.5f);
    ESP_LOGI(TAG, "ADC crudo: %d, ADC mV: %d, Bateria calculada: %d mV", raw_val, voltage_mv, battery_mv);

    return battery_mv;
}

int battery_get_percentage(void)
{
    int mv = battery_get_voltage_mv();
    int current_pct;

    if (mv >= 4200) {
        current_pct = 100;
    } else if (mv <= 3000) {
        current_pct = 0;
    } else {
        current_pct = ((mv - 3000) * 100) / (4200 - 3000);
    }

    if (smoothed_pct < 0) {
        smoothed_pct = (float)current_pct;
    } else {
        smoothed_pct = smoothed_pct * 0.9f + (float)current_pct * 0.1f;
    }

    return (int)(smoothed_pct + 0.5f);
}