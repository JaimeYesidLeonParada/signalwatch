# Tests unitarios en el PC (gcc + Unity). No usan ESP-IDF, ni FreeRTOS, ni la placa.
# Cada test compila los .c reales de ../main que prueba.
#
#   cmake -S tests -B tests/build -G Ninja
#   cmake --build tests/build
#   ctest --test-dir tests/build --output-on-failure
cmake_minimum_required(VERSION 3.16)
project(smartwatch_tests C)

set(CMAKE_C_STANDARD 11)
enable_testing()

add_library(unity STATIC unity/unity.c)
target_include_directories(unity PUBLIC unity)

set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../main)

# add_unit_test(<nombre> <fuentes de main a probar>...)
# Crea el ejecutable test_<nombre> desde test_<nombre>.c y lo registra en CTest.
function(add_unit_test name)
    set(sources "")
    foreach(file ${ARGN})
        list(APPEND sources ${MAIN_DIR}/${file})
    endforeach()
    add_executable(test_${name} test_${name}.c ${sources})
    target_include_directories(test_${name} PRIVATE ${MAIN_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
    target_link_libraries(test_${name} PRIVATE unity)
    target_compile_options(test_${name} PRIVATE -Wall -Wextra)
    add_test(NAME ${name} COMMAND test_${name})
endfunction()

add_unit_test(alert_module alert_module.c)
add_unit_test(alert_catalog alert_catalog.c)
add_unit_test(morse morse.c)
add_unit_test(rtc_codec rtc_codec.c)
add_unit_test(battery_math battery_math.c)
add_unit_test(clock_format clock_format.c)
add_unit_test(audio_level audio_level.c)
add_unit_test(audio_preroll audio_preroll.c)
add_unit_test(alert_dispatcher alert_dispatcher.c alert_module.c alert_catalog.c)
add_unit_test(time_edit time_edit.c)
add_unit_test(alert_urgent alert_urgent.c alert_catalog.c)
add_unit_test(screen_policy screen_policy.c)
