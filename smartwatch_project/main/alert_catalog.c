#include "alert_catalog.h"
#include <string.h>

// El orden es el orden de los iconos en pantalla, de izquierda a derecha.
static const alert_class_t CATALOG[] = {
    { "ambulance",   "A", "\xEF\x83\xB9",
      { "Posible ambulancia cerca",
        "Parece que viene una ambulancia",
        "Alerta! Ambulancia cerca" } },
    { "baby-crying", "B", "\xEF\x9D\xBC",
      { "Posible llanto de bebe",
        "Parece que un bebe esta llorando",
        "Alerta! Bebe llorando" } },
    { "fire",        "F", "\xEF\x81\xAD",
      { "Posible alarma de fuego",
        "Parece que suena una alarma de fuego",
        "Alerta! Alarma de fuego" } },
};
#define CATALOG_COUNT (sizeof(CATALOG) / sizeof(CATALOG[0]))

_Static_assert(CATALOG_COUNT <= ALERT_CATALOG_MAX, "alert_catalog: subir ALERT_CATALOG_MAX");

size_t alert_catalog_count(void)
{
    return CATALOG_COUNT;
}

const alert_class_t *alert_catalog_get(size_t index)
{
    return (index < CATALOG_COUNT) ? &CATALOG[index] : NULL;
}

int alert_catalog_index_of(const char *label)
{
    for (size_t i = 0; i < CATALOG_COUNT; i++) {
        if (strcmp(CATALOG[i].label, label) == 0) return (int)i;
    }
    return -1;
}
