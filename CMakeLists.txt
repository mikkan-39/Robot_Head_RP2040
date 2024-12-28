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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Robot_Head_RP2040 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Robot_Head_RP2040
src/Main.cpp
src/Core1.cpp
src/multicore/UART_Utils.cpp
src/multicore/CommandParsers.cpp
src/multicore/MulticoreUtils.cpp
src/gui/GUI_Paint.cpp
src/drivers/LCD_Driver.cpp
src/drivers/GpioUtils.cpp
src/drivers/TOF_Driver.cpp
src/drivers/IMU/Accelerometer.cpp
src/drivers/IMU/BaseIMU.cpp
src/drivers/IMU/Compass.cpp
src/drivers/IMU/Gyroscope.cpp
src/drivers/IMU/MadgwickAHRS.cpp 
)

target_include_directories(Robot_Head_RP2040 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

pico_generate_pio_header(Robot_Head_RP2040 ${CMAKE_CURRENT_LIST_DIR}/lcd.pio)

pico_set_program_name(Robot_Head_RP2040 "Robot_Head_RP2040")
pico_set_program_version(Robot_Head_RP2040 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Robot_Head_RP2040 0)
pico_enable_stdio_usb(Robot_Head_RP2040 1)

# Add the standard library to the build
target_link_libraries(Robot_Head_RP2040
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Robot_Head_RP2040 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Robot_Head_RP2040 
        hardware_spi
        hardware_i2c
        hardware_pio
        pico_rand
        pico_multicore
        )

pico_add_extra_outputs(Robot_Head_RP2040)

