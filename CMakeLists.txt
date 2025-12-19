# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(USERHOME $ENV{HOME})
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

# SD Card / FATFS library
FetchContent_Declare(
    sd_lib
    GIT_REPOSITORY https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico.git
    GIT_TAG bbccc5d2deea09e128e2576ef35f72b473c5ba5f
)
FetchContent_MakeAvailable(sd_lib)

# DS3231 library
FetchContent_Declare(
    ds3231_lib
    GIT_REPOSITORY https://github.com/antgon/pico-ds3231.git
    GIT_TAG 29805441b1a163da75ba0e65ea11471d186952bc
)
FetchContent_MakeAvailable(ds3231_lib)

project(dive_computer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add FatFs_SPI library
add_subdirectory(${sd_lib_SOURCE_DIR}/FatFs_SPI ${CMAKE_BINARY_DIR}/fatfs_build)

# Add DS3231 library
add_subdirectory(${ds3231_lib_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR}/ds323_build)

# Add executable
add_executable(${PROJECT_NAME}
    src/main.c
    src/ms5803_i2c/ms5803_i2c.c
    src/dive_logger/sd_hw_config.c
    src/dive_logger/dive_logger.c
    src/epd_waveshare_1in54/DEV_Config.c
    src/epd_waveshare_1in54/EPD_1in54_V2.c
    src/epd_waveshare_1in54/GUI/GUI_BMPfile.c
    src/epd_waveshare_1in54/GUI/GUI_Paint.c
    src/epd_waveshare_1in54/Fonts/font8.c
    src/epd_waveshare_1in54/Fonts/font12.c
    src/epd_waveshare_1in54/Fonts/font16.c
    src/epd_waveshare_1in54/Fonts/font20.c
    src/epd_waveshare_1in54/Fonts/font24.c
    src/epd_waveshare_1in54/dive_display.c
)

include_directories(src/include)
include_directories(src/ms5803/include)
include_directories(src/dive_logger/include)
include_directories(src/epd_waveshare_1in54/include)
include_directories(src/epd_waveshare_1in54/GUI/include)
include_directories(src/epd_waveshare_1in54/Fonts/include)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include/
)

# Pull in common dependencies
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    FatFs_SPI
    pico-ds3231
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)


# Add OpenOCD based pico program target
add_custom_target(flash
    COMMAND openocd 
        -s tcl 
        -f interface/cmsis-dap.cfg 
        -f target/rp2040.cfg 
        -c "adapter speed 5000" 
        -c "program ${PROJECT_NAME}.elf verify reset exit"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flashing ${PROJECT_NAME}.elf to RP2040 via OpenOCD..."
    USES_TERMINAL
)