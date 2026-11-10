idf_component_register(SRCS "date_font_28.c" "nine_pin_48.c" "alert_urgent.c" "ui_settings.c" "time_edit.c" "audio_preroll.c" "audio_level.c" "clock_format.c" "battery_math.c" "rtc_codec.c" "morse.c" "app_init.c" "i2c_bus.c" "ui_alerts.c" "ui_vu.c" "ui_notification.c" "ui_battery.c" "ui_clock.c" "display_hw.c" "alert_dispatcher.c" "ai_module.c" "main.c" "ui.c" "bsp_es8311.c" "rtc_module.c" "button_module.c" "battery_module.c" "touch_module.c" "ei_wrapper.cpp" "mini_system_80.c" "nine_pin_70.c" "vibrator_module.c" "alert_module.c" "alert_catalog.c" "fa_icons_32.c"
                    INCLUDE_DIRS ".")

list(APPEND EXTRA_COMPONENT_DIRS components)

# Relajamos las reglas estrictas de C++ para cuando main incluya a Edge Impulse
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-error" 
    "-Wno-missing-field-initializers" 
    "-Wno-deprecated-declarations"
)
