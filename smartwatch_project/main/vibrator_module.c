#include "vibrator_module.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdbool.h>

#define MOTOR_VIBRATOR_PIN     18
#define VIBRATOR_TASK_PRIORITY 2

#define MORSE_UNIT_MS        150                    // duracion de un "punto"
#define MORSE_DASH_MS        (MORSE_UNIT_MS * 3)     // duracion de una "raya"
#define MORSE_SYMBOL_GAP_MS  MORSE_UNIT_MS           // pausa entre simbolos de una letra
#define MORSE_LETTER_GAP_MS  (MORSE_UNIT_MS * 3)     // pausa entre letras
#define MORSE_WORD_GAP_MS    (MORSE_UNIT_MS * 7)     // pausa entre palabras (espacio)

#define VIBRATOR_MAX_MESSAGE_LEN 32

typedef char vibrator_message_t[VIBRATOR_MAX_MESSAGE_LEN];

static QueueHandle_t vibration_queue;
static volatile bool vibrator_busy = false;

static const char *const MORSE_LETTERS[26] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
};

static const char *const MORSE_DIGITS[10] = {
    "-----", ".----", "..---", "...--", "....-",
    ".....", "-....", "--...", "---..", "----.",
};

static const char *morse_for_char(char c)
{
    if (c >= 'a' && c <= 'z') c -= ('a' - 'A');
    if (c >= 'A' && c <= 'Z') return MORSE_LETTERS[c - 'A'];
    if (c >= '0' && c <= '9') return MORSE_DIGITS[c - '0'];
    return NULL; // caracter no soportado, se ignora
}

static void vibrator_play_symbol(char symbol)
{
    uint32_t on_ms = (symbol == '-') ? MORSE_DASH_MS : MORSE_UNIT_MS;
    gpio_set_level(MOTOR_VIBRATOR_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(on_ms));
    gpio_set_level(MOTOR_VIBRATOR_PIN, 0);
}

static void vibrator_task(void *arg)
{
    vibrator_message_t msg;
    while (1) {
        xQueueReceive(vibration_queue, msg, portMAX_DELAY);
        vibrator_busy = true;

        bool first_letter = true;
        for (int i = 0; msg[i] != '\0'; i++) {
            char c = msg[i];

            if (c == ' ') {
                vTaskDelay(pdMS_TO_TICKS(MORSE_WORD_GAP_MS));
                first_letter = true;
                continue;
            }

            const char *code = morse_for_char(c);
            if (code == NULL) continue;

            if (!first_letter) {
                vTaskDelay(pdMS_TO_TICKS(MORSE_LETTER_GAP_MS));
            }
            first_letter = false;

            for (int s = 0; code[s] != '\0'; s++) {
                vibrator_play_symbol(code[s]);
                if (code[s + 1] != '\0') {
                    vTaskDelay(pdMS_TO_TICKS(MORSE_SYMBOL_GAP_MS));
                }
            }
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