#ifndef VIBRATOR_MODULE_H
#define VIBRATOR_MODULE_H

void vibrator_init(void);
void vibrator_task_start(void);

// Convierte el texto a codigo morse y lo vibra letra por letra.
// Soporta A-Z (mayus o minuscula), 0-9, y espacio como separador de palabra.
// Si el motor esta ocupado, el mensaje se ignora.
void vibration_morse_play(const char *text);

#endif // VIBRATOR_MODULE_H