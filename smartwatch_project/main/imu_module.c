#include "imu_module.h"
#include "wrist_gesture.h"   // solo para el periodo de muestreo
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "imu";

// La direccion depende del pin SA0 del chip; se prueban las dos.
#define QMI8658_ADDR_HIGH   0x6B
#define QMI8658_ADDR_LOW    0x6A

#define REG_WHO_AM_I        0x00   // debe leerse 0x05
#define REG_CTRL1           0x02
#define REG_CTRL2           0x03
#define REG_CTRL7           0x08
#define REG_ACCEL_X_L       0x35   // X, Y, Z: 2 bytes cada uno, el bajo primero
#define WHO_AM_I_VALUE      0x05

#define CTRL1_ADDR_AUTO_INC 0x40   // permite leer los 6 bytes seguidos
#define CTRL2_2G_56HZ       0x07   // fondo de escala +-2 g, 56 Hz
#define CTRL7_ACCEL_ENABLE  0x01
#define LSB_PER_G           16384  // a +-2 g

// Como esta montado el sensor: signo de cada eje del chip para pasarlo a la convencion de
// wrist_gesture.h (az positivo con la pantalla hacia arriba). Se ajusta mirando los valores del
// log con el reloj en distintas posiciones.
#define IMU_AXIS_X_SIGN   (+1)
#define IMU_AXIS_Y_SIGN   (+1)
#define IMU_AXIS_Z_SIGN   (-1)   // el chip da -1 g con la pantalla hacia arriba (medido en el reloj)

// Mientras se ajustan los ejes: cada cuantas muestras se escribe una al log (0 = nunca).
#define IMU_LOG_EVERY_N   0

static i2c_master_dev_handle_t dev;
static imu_sample_cb_t sample_cb;
static TaskHandle_t task_handle;
static volatile bool enabled;

static bool read_regs(uint8_t reg, uint8_t *out, size_t len)
{
    return i2c_master_transmit_receive(dev, &reg, 1, out, len, -1) == ESP_OK;
}

static bool write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    return i2c_master_transmit(dev, buf, sizeof(buf), -1) == ESP_OK;
}

static int raw_to_mg(const uint8_t *p)
{
    int16_t raw = (int16_t)(((uint16_t)p[1] << 8) | p[0]);
    return (int)(((int32_t)raw * 1000) / LSB_PER_G);
}

bool imu_init(i2c_master_bus_handle_t bus_handle)
{
    static const uint8_t addrs[] = { QMI8658_ADDR_HIGH, QMI8658_ADDR_LOW };
    uint8_t found = 0;
    for (size_t i = 0; i < sizeof(addrs); i++) {
        if (i2c_master_probe(bus_handle, addrs[i], 50) == ESP_OK) {
            found = addrs[i];
            break;
        }
    }
    if (found == 0) {
        ESP_LOGW(TAG, "acelerometro no encontrado (0x%02X ni 0x%02X)", QMI8658_ADDR_HIGH, QMI8658_ADDR_LOW);
        return false;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = found,
        .scl_speed_hz = 400000,
    };
    if (i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev) != ESP_OK) {
        ESP_LOGW(TAG, "no se pudo agregar el acelerometro al bus");
        dev = NULL;
        return false;
    }

    uint8_t who = 0;
    if (!read_regs(REG_WHO_AM_I, &who, 1) || who != WHO_AM_I_VALUE) {
        ESP_LOGW(TAG, "0x%02X responde pero no es un QMI8658 (WHO_AM_I=0x%02X)", found, who);
        dev = NULL;
        return false;
    }

    if (!write_reg(REG_CTRL1, CTRL1_ADDR_AUTO_INC) ||
        !write_reg(REG_CTRL2, CTRL2_2G_56HZ) ||
        !write_reg(REG_CTRL7, CTRL7_ACCEL_ENABLE)) {
        ESP_LOGW(TAG, "fallo al configurar el acelerometro");
        dev = NULL;
        return false;
    }
    ESP_LOGI(TAG, "QMI8658 listo en 0x%02X", found);
    return true;
}

static void imu_task(void *arg)
{
    int count = 0;
    while (1) {
        if (!enabled) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // duerme hasta que lo activen
            count = 0;
            continue;
        }

        uint8_t raw[6];
        if (read_regs(REG_ACCEL_X_L, raw, sizeof(raw))) {
            int ax = IMU_AXIS_X_SIGN * raw_to_mg(&raw[0]);
            int ay = IMU_AXIS_Y_SIGN * raw_to_mg(&raw[2]);
            int az = IMU_AXIS_Z_SIGN * raw_to_mg(&raw[4]);
            if (IMU_LOG_EVERY_N > 0 && ++count % IMU_LOG_EVERY_N == 0) {
                ESP_LOGI(TAG, "ax=%d ay=%d az=%d mg", ax, ay, az);
            }
            if (sample_cb != NULL) {
                sample_cb(ax, ay, az);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(WRIST_GESTURE_SAMPLE_PERIOD_MS));
    }
}

void imu_start(imu_sample_cb_t on_sample)
{
    if (dev == NULL) return;
    sample_cb = on_sample;
    xTaskCreate(imu_task, "imu_task", 3072, NULL, 2, &task_handle);
}

void imu_set_enabled(bool on)
{
    enabled = on;
    if (on && task_handle != NULL) {
        xTaskNotifyGive(task_handle);
    }
}
