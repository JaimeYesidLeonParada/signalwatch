idf_component_register(SRCS "main.c" "ui.c" "bsp_es8311.c" "vuvometro.c" "gpio_config.c" "rtc_module.c" "button_module.c" "battery_module.c" "touch_module.c" "ei_wrapper.cpp"
                    INCLUDE_DIRS ".")

list(APPEND EXTRA_COMPONENT_DIRS components)

# Relajamos las reglas estrictas de C++ para cuando main incluya a Edge Impulse
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-error" 
    "-Wno-missing-field-initializers" 
    "-Wno-deprecated-declarations"
)
