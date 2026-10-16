#pragma once

#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_codec_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

void bsp_es8311_init(i2c_master_bus_handle_t bus_handle);

// Handle del microfono; valido solo despues de bsp_es8311_init().
esp_codec_dev_handle_t bsp_es8311_get_input_dev(void);
void bsp_es8311_test(void);

void bsp_es8311_recording(uint8_t *data, size_t limit_size);
void bsp_es8311_playing(uint8_t *data, size_t limit_size);

void bsp_es8311_set_out_volume(float volume);
void bsp_es8311_set_in_volume(float volume);

#ifdef __cplusplus
}
#endif