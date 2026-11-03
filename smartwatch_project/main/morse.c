#include "morse.h"

static const char *const MORSE_LETTERS[26] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
};

static const char *const MORSE_DIGITS[10] = {
    "-----", ".----", "..---", "...--", "....-",
    ".....", "-....", "--...", "---..", "----.",
};

const char *morse_for_char(char c)
{
    if (c >= 'a' && c <= 'z') c -= ('a' - 'A');
    if (c >= 'A' && c <= 'Z') return MORSE_LETTERS[c - 'A'];
    if (c >= '0' && c <= '9') return MORSE_DIGITS[c - '0'];
    return NULL; // caracter no soportado, se ignora
}

static bool push_step(morse_step_t *steps, size_t *count, size_t max_steps, bool on, uint16_t ms)
{
    if (*count >= max_steps) return false;
    steps[*count].on = on;
    steps[*count].ms = ms;
    (*count)++;
    return true;
}

size_t morse_encode(const char *text, morse_step_t *steps, size_t max_steps)
{
    size_t count = 0;
    bool first_letter = true;

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];

        if (c == ' ') {
            if (!push_step(steps, &count, max_steps, false, MORSE_WORD_GAP_MS)) return count;
            first_letter = true;
            continue;
        }

        const char *code = morse_for_char(c);
        if (code == NULL) continue;

        if (!first_letter) {
            if (!push_step(steps, &count, max_steps, false, MORSE_LETTER_GAP_MS)) return count;
        }
        first_letter = false;

        for (int s = 0; code[s] != '\0'; s++) {
            if (!push_step(steps, &count, max_steps, true, (code[s] == '-') ? MORSE_DASH_MS : MORSE_UNIT_MS)) return count;
            if (code[s + 1] != '\0') {
                if (!push_step(steps, &count, max_steps, false, MORSE_SYMBOL_GAP_MS)) return count;
            }
        }
    }

    return count;
}
