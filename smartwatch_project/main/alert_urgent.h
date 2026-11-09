#ifndef ALERT_URGENT_H
#define ALERT_URGENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Confianza por encima de la cual una deteccion es tan clara que justifica interrumpir lo
// que el usuario este haciendo en el reloj (por ejemplo, salir de la pantalla de ajustes).
#define ALERT_URGENT_CONFIDENCE 0.90f

// true si label es una clase del catalogo de alertas y confidence (0..1) supera
// ALERT_URGENT_CONFIDENCE. Las etiquetas que no son alertas (ruido de fondo, silencio) nunca
// son urgentes, por segura que sea la deteccion. El "ruido fuerte" no es una clasificacion
// del modelo y tampoco pasa por aqui.
bool alert_is_urgent(const char *label, float confidence);

#ifdef __cplusplus
}
#endif

#endif // ALERT_URGENT_H
