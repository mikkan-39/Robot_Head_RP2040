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

project(Robot_Eyes C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Robot_Eyes
src/Main.cpp
src/Core1.cpp
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

pico_generate_pio_header(Robot_Eyes ${CMAKE_CURRENT_LIST_DIR}/lcd.pio)

pico_set_program_name(Robot_Eyes "Robot_Eyes")
pico_set_program_version(Robot_Eyes "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Robot_Eyes 1)
pico_enable_stdio_usb(Robot_Eyes 0)

# Add the standard library to the build
target_link_libraries(Robot_Eyes
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Robot_Eyes PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Robot_Eyes 
        hardware_spi
        hardware_i2c
        hardware_pio
        pico_rand
        pico_multicore
        )

pico_add_extra_outputs(Robot_Eyes)

