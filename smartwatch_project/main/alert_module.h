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

void alert_module_init(void);

// Agrega una lectura de confianza (0.0-1.0) para una clase, y devuelve
// el nivel post-procesado (promedio de las ultimas muestras) para esa clase.
alert_level_t alert_module_add_sample(const char *label, float confidence);

#ifdef __cplusplus
}
#endif

#endif // ALERT_MODULE_H