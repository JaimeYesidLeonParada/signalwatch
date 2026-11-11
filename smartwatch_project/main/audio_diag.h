#ifndef AUDIO_DIAG_H
#define AUDIO_DIAG_H

#include <stdint.h>
#include "ei_wrapper.h"

// Diagnostico del audio que oye el reloj, para entender por que una clase falla:
//   1. En cada ventana clasificada escribe al log su pico, su nivel medio y cuantas muestras
//      estan saturadas (el microfono llego al tope y deformo la senal).
//   2. Si AUDIO_DIAG_DUMP esta en 1, ademas vuelca por el puerto serie el audio de una ventana
//      cada AUDIO_DIAG_DUMP_INTERVAL_MS, en base64, para convertirlo en un WAV con
//      tools/audiodump_to_wav.py y escucharlo, analizarlo o subirlo a Edge Impulse.
//
// Es solo una herramienta de diagnostico: no cambia lo que hace el reloj.

// 1 = volcar el audio por serie (genera mucho texto en el monitor). 0 = solo las estadisticas.
#define AUDIO_DIAG_DUMP              0
#define AUDIO_DIAG_DUMP_INTERVAL_MS  8000

// Reserva el buffer del volcado y lanza su tarea. Llamar una vez antes de recibir ventanas.
void audio_diag_init(void);

// Una ventana de audio ya clasificada, con su resultado. Cumple on_window de ai_listener_t.
// samples solo es valido durante la llamada: el volcado hace su propia copia.
void audio_diag_on_window(const int16_t *samples, int n, const ei_wrapper_result_t *result);

#endif // AUDIO_DIAG_H
