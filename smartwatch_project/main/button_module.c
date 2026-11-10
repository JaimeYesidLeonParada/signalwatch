#include "button_module.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define BOOT_BUTTON_PIN 9

static const char *TAG = "button_module";
static button_cb_t press_cb;

static void button_task(void *arg)
{
    // El botón tiene resistencia Pull-up, así que 1 es suelto y 0 es presionado
    int last_state = 1; 
    
    while (1) {
        int current_state = gpio_get_level(BOOT_BUTTON_PIN);
        
        // Si el estado actual es 0 (presionado) y el anterior era 1 (suelto)
        if (current_state == 0 && last_state == 1) {
            ESP_LOGI(TAG, "¡Botón BOOT presionado!");

            if (press_cb != NULL) {
                press_cb();
            }
        }
        
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50)); // Pausa de 50ms para evitar falsos positivos
    }
}

void button_init(button_cb_t on_press)
{
    press_cb = on_press;
    ESP_LOGI(TAG, "Inicializando botón BOOT...");
    
    // Configuramos el pin 9 como entrada con Pull-up interno
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << BOOT_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_cfg);

    // Lanzamos la tarea de monitoreo del botón
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
}