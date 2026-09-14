# 1. Buscamos de forma recursiva TODOS los archivos .cpp y .c necesarios
file(GLOB_RECURSE ei_srcs
    "tflite-model/*.cpp"
    "tflite-model/*.c"
    "tflite-model/*.cc"
    "edge-impulse-sdk/classifier/*.cpp"
    "edge-impulse-sdk/classifier/*.c"
    "edge-impulse-sdk/classifier/*.cc"
    "edge-impulse-sdk/dsp/*.cpp"
    "edge-impulse-sdk/dsp/*.c"
    "edge-impulse-sdk/dsp/*.cc"
    "edge-impulse-sdk/porting/*.cpp"
    "edge-impulse-sdk/porting/*.c"
    "edge-impulse-sdk/porting/*.cc"
    "edge-impulse-sdk/tensorflow/*.cpp"
    "edge-impulse-sdk/tensorflow/*.c"
    "edge-impulse-sdk/tensorflow/*.cc"
)

# 2. Registramos el componente.
# El truco vital aquí es INCLUDE_DIRS "." (el punto significa la raíz del componente)
idf_component_register(SRCS ${ei_srcs}
                       INCLUDE_DIRS "." 
                                    "model-parameters" 
                                    "tflite-model")

# 3. Optimizamos para tamaño y apagamos las advertencias estrictas del compilador
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Os" 
    "-Wno-error" 
    "-Wno-missing-field-initializers" 
    "-Wno-deprecated-declarations"
)