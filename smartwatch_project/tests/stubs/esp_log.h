#ifndef STUB_ESP_LOG_H
#define STUB_ESP_LOG_H

// Reemplazo de esp_log.h para compilar en el PC: no imprime nada, pero sigue usando los
// argumentos (asi no aparecen avisos de variables sin usar).
static inline void esp_log_ignore(const char *tag, const char *fmt, ...)
{
    (void)tag;
    (void)fmt;
}

#define ESP_LOGE(tag, fmt, ...) esp_log_ignore(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) esp_log_ignore(tag, fmt, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) esp_log_ignore(tag, fmt, ##__VA_ARGS__)

#endif // STUB_ESP_LOG_H
