#ifndef MORSE_H
#define MORSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MORSE_UNIT_MS        150                    // duracion de un "punto"
#define MORSE_DASH_MS        (MORSE_UNIT_MS * 3)     // duracion de una "raya"
#define MORSE_SYMBOL_GAP_MS  MORSE_UNIT_MS           // pausa entre simbolos de una letra
#define MORSE_LETTER_GAP_MS  (MORSE_UNIT_MS * 3)     // pausa entre letras
#define MORSE_WORD_GAP_MS    (MORSE_UNIT_MS * 7)     // pausa entre palabras (espacio)

// Peor caso por caracter: un digito (5 vibraciones + 4 pausas entre simbolos) mas la pausa de letra.
#define MORSE_STEPS_PER_CHAR 10

// Un tramo de la secuencia: vibrar (on) o esperar (!on) durante ms.
typedef struct {
    uint16_t ms;
    bool on;
} morse_step_t;

// Codigo morse ("." y "-") de un caracter: A-Z (mayus o minuscula) y 0-9.
// Devuelve NULL si el caracter no tiene codigo.
const char *morse_for_char(char c);

// Convierte el texto en la secuencia de vibraciones y pausas, y devuelve cuantos tramos escribio.
// El espacio separa palabras; los caracteres sin codigo se ignoran. Si steps se llena, corta ahi.
size_t morse_encode(const char *text, morse_step_t *steps, size_t max_steps);

#ifdef __cplusplus
}
#endif

#endif // MORSE_H
