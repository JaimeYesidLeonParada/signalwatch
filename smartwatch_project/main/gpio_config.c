#include "gpio_config.h"
#include "driver/gpio.h"

#define MOTOR_VIBRATOR_PIN 18
#define BATTERY_ENABLE_PIN 15

void gpio_config_init(void)
{
    gpio_reset_pin(MOTOR_VIBRATOR_PIN);
    gpio_set_direction(MOTOR_VIBRATOR_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BATTERY_ENABLE_PIN);
    gpio_set_direction(BATTERY_ENABLE_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(MOTOR_VIBRATOR_PIN, 0); // Motor apagado
    gpio_set_level(BATTERY_ENABLE_PIN, 1); // Batería habilitada
}