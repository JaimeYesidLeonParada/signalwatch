idf_component_register(SRCS "ai_module.c" "main.c" "ui.c" "bsp_es8311.c" "gpio_config.c" "rtc_module.c" "button_module.c" "battery_module.c" "touch_module.c" "ei_wrapper.cpp" "mini_system_80.c" "nine_pin_70.c"
                    INCLUDE_DIRS ".")

list(APPEND EXTRA_COMPONENT_DIRS components)

# Relajamos las reglas estrictas de C++ para cuando main incluya a Edge Impulse
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-error" 
    "-Wno-missing-field-initializers" 
    "-Wno-deprecated-declarations"
)
