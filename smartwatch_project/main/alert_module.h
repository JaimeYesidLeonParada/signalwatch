#ifndef ALERT_MODULE_H
#define ALERT_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ALERT_NONE,
    ALERT_LOW,
    ALERT_MEDIUM,
    ALERT_HIGH,
} alert_level_t;

#include <stdint.h>

// Una muestra mas vieja que esto no cuenta para el promedio. Las muestras solo llegan cuando
// hay sonido, asi que sin esto, despues de un rato en silencio un sonido debil se promediaria
// con lecturas altas de hace minutos y daria una alerta falsa. Con una clasificacion cada ~2 s,
// 10 s cubren las 3 muestras de la ventana con margen.
#define ALERT_SAMPLE_MAX_AGE_MS 10000u

void alert_module_init(void);

// Agrega una lectura de confianza (0.0-1.0) para una clase, y devuelve el nivel
// post-procesado (promedio de las ultimas muestras recientes) para esa clase.
// now_ms es el momento de la lectura en milisegundos; solo importan las diferencias.
alert_level_t alert_module_add_sample(const char *label, float confidence, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif // ALERT_MODULE_H