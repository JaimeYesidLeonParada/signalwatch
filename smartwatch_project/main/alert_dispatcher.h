#ifndef ALERT_DISPATCHER_H
#define ALERT_DISPATCHER_H

#include "ei_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

// Politica de alertas: decide que mostrar y cuando vibrar a partir de lo que oye el reloj.
// Es el unico lugar que conecta el audio con ui, vibrator, alert_module y alert_catalog.
void alert_dispatcher_init(void);

// Nivel de audio (0-100) de cada chunk capturado. Debe llamarse siempre desde la
// misma tarea, porque el dispatcher guarda el estado del flanco de "ruido fuerte".
void alert_dispatcher_on_audio_level(int level_pct);

// Resultado de una clasificacion del modelo.
void alert_dispatcher_on_classification(const ei_wrapper_result_t *result);

#ifdef __cplusplus
}
#endif

#endif // ALERT_DISPATCHER_H
