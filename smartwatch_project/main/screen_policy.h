#ifndef SCREEN_POLICY_H
#define SCREEN_POLICY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Cuando debe estar encendida la pantalla: logica pura, sin backlight, LVGL, touch, botones ni
// acelerometro. Quien la use solo le avisa de la actividad (de donde venga) y del paso del
// tiempo, y ejecuta lo que ella decide. Asi una fuente nueva de despertar (la muneca, por
// ejemplo) no cambia esta logica.
//
// El tiempo se pasa como "ahora" en milisegundos (por ejemplo esp_timer o xTaskGetTickCount).
// Solo importan las diferencias, asi que da igual de donde empiece o que el contador de 32 bits
// de la vuelta.

// Tiempo por defecto sin actividad antes de apagar la pantalla.
#define SCREEN_POLICY_DEFAULT_TIMEOUT_MS 30000u

typedef struct {
    bool on;                  // estado decidido: true = pantalla encendida
    uint32_t timeout_ms;      // 0 = nunca se apaga
    uint32_t last_activity_ms;
} screen_policy_t;

// Empieza con la pantalla encendida y la actividad en now_ms.
void screen_policy_init(screen_policy_t *p, uint32_t timeout_ms, uint32_t now_ms);

// Hubo actividad: toque, boton, gesto de muneca o una alerta. Enciende la pantalla (si estaba
// apagada) y reinicia la cuenta. Devuelve true si la pantalla estaba apagada y acaba de
// encenderse: quien recibio un toque o un boton puede usarlo para que ese primer toque solo
// despierte y no pulse nada.
bool screen_policy_activity(screen_policy_t *p, uint32_t now_ms);

// Se llama cada cierto tiempo. Apaga la pantalla si ya paso el timeout sin actividad. Devuelve
// true si la pantalla acaba de apagarse (el unico momento en que hay que apagar el backlight).
bool screen_policy_tick(screen_policy_t *p, uint32_t now_ms);

// Estado actual: true = encendida.
bool screen_policy_is_on(const screen_policy_t *p);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_POLICY_H
