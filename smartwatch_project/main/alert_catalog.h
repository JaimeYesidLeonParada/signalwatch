#ifndef ALERT_CATALOG_H
#define ALERT_CATALOG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tope de clases; los modulos que guardan estado por clase dimensionan sus arreglos con esto.
#define ALERT_CATALOG_MAX 8

// Una clase de alerta que el reloj sabe anunciar. Para agregar una, basta una entrada en
// alert_catalog.c (y el glifo, si no existe todavia en fa_icons_32).
typedef struct {
    const char *label;   // etiqueta exacta que devuelve el modelo
    const char *morse;   // texto que vibra vibration_morse_play()
    const char *glyph;   // icono (UTF-8) de la fuente fa_icons_32
    const char *msg[3];  // texto en pantalla: [0]=LOW, [1]=MEDIUM, [2]=HIGH
} alert_class_t;

size_t alert_catalog_count(void);
const alert_class_t *alert_catalog_get(size_t index);

// Indice de la clase con esa etiqueta, o -1 si no esta en el catalogo.
int alert_catalog_index_of(const char *label);

#ifdef __cplusplus
}
#endif

#endif // ALERT_CATALOG_H
