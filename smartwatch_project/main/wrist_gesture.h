#ifndef WRIST_GESTURE_H
#define WRIST_GESTURE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Detector del gesto "levantar la muneca para mirar la hora": logica pura, sin acelerometro,
// I2C ni FreeRTOS. Quien lo use le pasa las muestras de aceleracion a un ritmo fijo y el
// detector dice cuando ocurrio el gesto.
//
// Convenciones de las muestras (las pone el driver del sensor, que conoce como esta montado):
//   - Unidades: milli-g (1000 = 1 g).
//   - az es el eje perpendicular a la pantalla y es positivo cuando la pantalla mira hacia
//     arriba; ax y ay son los ejes de la pantalla.
//
// El gesto es: la pantalla estaba "de lado o hacia abajo" (brazo caido) y en poco tiempo pasa a
// "mirando hacia arriba, inclinada hacia la cara" y se queda quieta un momento. Mirar el reloj
// ya levantado, o dejarlo sobre una mesa, no cuenta: antes tiene que haber una pose de brazo
// caido.

// Cada cuanto llega una muestra (el driver debe respetarlo; ~25 Hz).
#define WRIST_GESTURE_SAMPLE_PERIOD_MS 40

// Poses, en milli-g del eje az. Valores iniciales razonables: se afinan probando en el reloj.
#define WRIST_AWAY_MAX_MG        250   // az <= esto: brazo caido (pantalla de lado o hacia abajo)
#define WRIST_LOOK_MIN_MG        500   // az >= esto: pantalla mirando hacia arriba/la cara
// La muestra solo cuenta si el reloj esta casi quieto: el modulo de la aceleracion debe estar
// cerca de 1 g. Durante el movimiento brusco del brazo no se decide nada.
#define WRIST_STEADY_MIN_MG      800
#define WRIST_STEADY_MAX_MG      1250
// Tiempo que hay que mantener la pose de mirar, y tiempo maximo desde el brazo caido.
#define WRIST_HOLD_MS            300
#define WRIST_RAISE_WINDOW_MS    1500

typedef struct {
    bool armed;               // se vio el brazo caido y aun no se ha disparado desde entonces
    uint32_t since_away_ms;   // tiempo desde la ultima muestra de brazo caido (satura)
    uint32_t hold_ms;         // tiempo seguido mirando y quieto
} wrist_gesture_t;

void wrist_gesture_init(wrist_gesture_t *g);

// Procesa una muestra. Devuelve true en el momento en que se completa el gesto (una sola vez
// por levantamiento: para volver a dispararse hay que bajar el brazo otra vez).
bool wrist_gesture_feed(wrist_gesture_t *g, int ax_mg, int ay_mg, int az_mg);

#ifdef __cplusplus
}
#endif

#endif // WRIST_GESTURE_H
