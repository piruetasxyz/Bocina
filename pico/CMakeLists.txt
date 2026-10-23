cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Bocina C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Build the reusable Bocina library
add_library(Bocina
    ./../src/Bocina.cpp
    ./PicoHardware.cpp
)

# generar bocina_i2s.pio.h a partir del programa pio
pico_generate_pio_header(Bocina ${CMAKE_CURRENT_LIST_DIR}/bocina_i2s.pio)

target_include_directories(Bocina
    PUBLIC
        ./../src
)

target_link_libraries(Bocina
    PUBLIC
        pico_stdlib
        hardware_clocks
        hardware_dma
        hardware_irq
        hardware_pio
        hardware_pwm
)

# Build the example firmware
# cada ejemplo es una carpeta con un main.cpp
function(agregar_ejemplo nombre)
    add_executable(${nombre}
        ${nombre}/main.cpp
    )

    target_link_libraries(${nombre}
        Bocina
        pico_stdlib
    )

    pico_set_program_name(${nombre} "${nombre}")
    pico_set_program_version(${nombre} "0.0.1 (2026-10-21)")

    pico_enable_stdio_uart(${nombre} 0)
    pico_enable_stdio_usb(${nombre} 1)

    pico_add_extra_outputs(${nombre})
endfunction()

agregar_ejemplo(ej00_sonar)
agregar_ejemplo(ej01_sinBloquear)
agregar_ejemplo(ej02_formasDeOnda)
agregar_ejemplo(ej03_grabacion)
