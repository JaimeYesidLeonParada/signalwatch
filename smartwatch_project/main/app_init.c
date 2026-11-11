#include "app_init.h"
#include "i2c_bus.h"
#include "ei_wrapper.h"
#include "bsp_es8311.h"
#include "rtc_module.h"
#include "display_hw.h"
#include "ui.h"
#include "button_module.h"
#include "battery_module.h"
#include "touch_module.h"
#include "ai_module.h"
#include "alert_dispatcher.h"
#include "vibrator_module.h"
#include "imu_module.h"
#include "wrist_wake.h"

#define BUTTON_PULSE_MS 100

// Feedback del boton con la pantalla encendida: una vibracion corta.
static void button_feedback(void)
{
    vibrator_pulse(BUTTON_PULSE_MS);
}

// Enciende o apaga la pantalla. Con ella apagada se vigila el gesto de muneca para volver a
// encenderla; con ella encendida el acelerometro duerme.
static void screen_power(bool on)
{
    if (!on) {
        wrist_wake_reset();   // hay que ver el brazo caido antes de aceptar un gesto
    }
    display_hw_set_backlight(on);
    imu_set_enabled(!on);
}

// Composition root: aqui se crean los modulos y se conectan entre si. El orden
// importa, cada paso indica de que depende.
void app_init(void)
{
    // Lo primero de todo: sin esto la placa no se alimenta de la bateria.
    battery_power_enable();

    // Pantalla y UI primero: los avisos de reloj y bateria llegan a la cola que crea
    // ui_start, y touch necesita el display ya inicializado.
    lv_disp_t *disp = display_hw_init();
    ui_start(disp);
    ui_set_display_power(screen_power);   // la UI apaga la pantalla si no se usa

    i2c_master_bus_handle_t bus_handle = i2c_bus_init();

    // Dispositivos del bus I2C
    bsp_es8311_init(bus_handle);
    bsp_es8311_set_in_volume(40.0);
    rtc_init(bus_handle);
    ui_set_time_source(rtc_read_wall_time);   // los ajustes leen la hora del RTC...
    ui_set_time_sink(rtc_set_time);          // ...y guardan en el RTC la que el usuario edita
    touch_init(bus_handle, disp);

    // Gesto de muneca: el acelerometro entrega muestras al detector solo con la pantalla apagada,
    // y el gesto enciende la pantalla igual que una alerta. Si el sensor no esta, no pasa nada.
    wrist_wake_init(ui_post_wake_msg);   // necesita la cola de ui
    if (imu_init(bus_handle)) {
        imu_start(wrist_wake_on_sample);
    }

    ui_set_button_feedback(button_feedback);
    button_init(ui_post_button_msg);   // necesita la cola de ui
    battery_init();
    battery_start(ui_post_battery_msg);   // necesita la cola de ui
    vibrator_init();

    // El dispatcher traduce los eventos de ai_module a UI y vibracion; debe existir
    // antes de que arranque la captura. Sus salidas necesitan la cola de ui y el vibrador.
    ei_wrapper_init();
    const alert_outputs_t alert_outputs = {
        .audio_level   = ui_post_audio_level_msg,
        .inference     = ui_post_inference_msg,
        .alert_icon    = ui_post_alert_icon_msg,
        .notification  = ui_post_notification_msg,
        .vibrate_morse = vibration_morse_play,
        .wake_screen   = ui_post_wake_msg,
    };
    alert_dispatcher_init(&alert_outputs);
    const ai_listener_t ai_listener = {
        .on_audio_level    = alert_dispatcher_on_audio_level,
        .on_classification = alert_dispatcher_on_classification,
    };
    ai_module_start(bsp_es8311_get_input_dev(), &ai_listener);

    rtc_task_start(ui_post_clock_msg);    // necesita la cola de ui
    vibrator_task_start();
}
