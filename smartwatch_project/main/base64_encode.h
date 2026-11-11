#ifndef BASE64_ENCODE_H
#define BASE64_ENCODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tamano que necesita el buffer de salida para codificar len bytes (incluye el '\0' final).
#define BASE64_ENCODED_SIZE(len) (4 * (((len) + 2) / 3) + 1)

// Codifica len bytes en base64 (RFC 4648, con relleno '=') y deja un '\0' al final.
// out debe tener al menos BASE64_ENCODED_SIZE(len) bytes. Devuelve cuantos caracteres escribio
// (sin contar el '\0').
size_t base64_encode(const uint8_t *in, size_t len, char *out);

#ifdef __cplusplus
}
#endif

#endif // BASE64_ENCODE_H
