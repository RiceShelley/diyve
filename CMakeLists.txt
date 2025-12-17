# Generated Cmake Pico project file

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
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

include(FetchContent)

FetchContent_Declare(
    tm1637_lib
    GIT_REPOSITORY https://github.com/wahlencraft/TM1637-pico.git # The GitHub URL
    GIT_TAG master       # Or a specific commit hash, tag, or branch (e.g., v1.0.0)
)

FetchContent_MakeAvailable(tm1637_lib)

project(dive_computer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include(${tm1637_lib_SOURCE_DIR}/PicoTM1637.cmake)

# Add executable. Default name is the project name, version 0.1

add_executable(${PROJECT_NAME}
    src/main.c
    src/ms5803_i2c/ms5803_i2c.c
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    src/ms5803/include/ms5803_i2c.h
    ${tm1637_lib_SOURCE_DIR}/src
)

# pull in common dependencies
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    PicoTM1637
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)