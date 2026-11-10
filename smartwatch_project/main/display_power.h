#ifndef DISPLAY_POWER_H
#define DISPLAY_POWER_H

#include <stdbool.h>

// Encender o apagar la pantalla, sin decir como. Hoy lo implementa display_hw con el backlight;
// si mas adelante tambien se duerme el panel, quien lo usa no cambia.
typedef void (*display_power_t)(bool on);

#endif // DISPLAY_POWER_H
