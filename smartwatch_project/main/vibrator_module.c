#include "vibrator_module.h"
#include "morse.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>

#define MOTOR_VIBRATOR_PIN     18
#define VIBRATOR_TASK_PRIORITY 2

#define VIBRATOR_MAX_MESSAGE_LEN 32

typedef char vibrator_message_t[VIBRATOR_MAX_MESSAGE_LEN];

static QueueHandle_t vibration_queue;
static volatile bool vibrator_busy = false;

void vibrator_pulse(uint32_t ms)
{
    gpio_set_level(MOTOR_VIBRATOR_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(MOTOR_VIBRATOR_PIN, 0);
}

static void vibrator_task(void *arg)
{
    vibrator_message_t msg;
    static morse_step_t steps[(VIBRATOR_MAX_MESSAGE_LEN - 1) * MORSE_STEPS_PER_CHAR]; // static: no gasta stack
    while (1) {
        xQueueReceive(vibration_queue, msg, portMAX_DELAY);
        vibrator_busy = true;

        size_t count = morse_encode(msg, steps, sizeof(steps) / sizeof(steps[0]));
        for (size_t i = 0; i < count; i++) {
            if (steps[i].on) vibrator_pulse(steps[i].ms);
            else             vTaskDelay(pdMS_TO_TICKS(steps[i].ms));
        }

        vibrator_busy = false;
    }
}

void vibrator_init(void)
{
    gpio_reset_pin(MOTOR_VIBRATOR_PIN);
    gpio_set_direction(MOTOR_VIBRATOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR_VIBRATOR_PIN, 0);

    vibration_queue = xQueueCreate(1, sizeof(vibrator_message_t));
}

void vibrator_task_start(void)
{
    xTaskCreate(vibrator_task, "vibrator_task", 2048, NULL, VIBRATOR_TASK_PRIORITY, NULL);
}

void vibration_morse_play(const char *text)
{
    if (vibrator_busy) {
        return;
    }

    vibrator_message_t buf;
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    xQueueSend(vibration_queue, buf, 0);
}