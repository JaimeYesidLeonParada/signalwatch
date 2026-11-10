#ifndef ALERT_DISPATCHER_H
#define ALERT_DISPATCHER_H

#include "ei_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

// A donde van las decisiones del dispatcher. app_init.c las conecta con ui y vibrator_module;
// una salida en NULL se ignora.
typedef struct {
    void (*audio_level)(int level_pct);                      // nivel de cada chunk, tal cual llega
    void (*inference)(const char *label, float confidence);  // la mejor clase de cada clasificacion
    void (*alert_icon)(const char *label, int level);        // nivel (alert_level_t) de cada clase
    void (*notification)(const char *text);                  // texto suelto en pantalla
    void (*vibrate_morse)(const char *text);                 // texto a vibrar en morse
    void (*wake_screen)(void);                               // una alerta de clase pide la pantalla encendida
} alert_outputs_t;

// Politica de alertas: decide que mostrar y cuando vibrar a partir de lo que oye el reloj.
// Une alert_module y alert_catalog con las salidas que recibe; no conoce ui ni el vibrador.
// outputs se copia, no hace falta que siga vivo despues de la llamada.
void alert_dispatcher_init(const alert_outputs_t *outputs);

// Nivel de audio (0-100) de cada chunk capturado. Debe llamarse siempre desde la
// misma tarea, porque el dispatcher guarda el estado del flanco de "ruido fuerte".
void alert_dispatcher_on_audio_level(int level_pct);

// Resultado de una clasificacion del modelo.
void alert_dispatcher_on_classification(const ei_wrapper_result_t *result);

#ifdef __cplusplus
}
#endif

#endif // ALERT_DISPATCHER_H
