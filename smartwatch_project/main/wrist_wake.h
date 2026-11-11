#ifndef WRIST_WAKE_H
#define WRIST_WAKE_H

// Une las muestras del acelerometro con el detector del gesto (wrist_gesture) y avisa cuando
// el usuario levanta la muneca. No conoce el sensor ni la pantalla: recibe muestras y avisa
// por callback.

typedef void (*wrist_wake_cb_t)(void);

// on_gesture se llama (desde la tarea que entrega las muestras) cada vez que se detecta el gesto.
void wrist_wake_init(wrist_wake_cb_t on_gesture);

// Olvida lo visto hasta ahora: hay que volver a ver el brazo caido antes de un nuevo gesto.
// Se llama al empezar a vigilar, con el sensor aun desactivado.
void wrist_wake_reset(void);

// Una muestra del acelerometro, en milli-g (ver wrist_gesture.h para la convencion de ejes).
void wrist_wake_on_sample(int ax_mg, int ay_mg, int az_mg);

#endif // WRIST_WAKE_H
