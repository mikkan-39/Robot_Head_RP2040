add_library(pico_vl53l0x STATIC
    src/VL53L0X.cpp
)

target_include_directories(pico_vl53l0x PUBLIC include)

target_link_libraries(pico_vl53l0x PUBLIC
    pico_stdlib
    hardware_i2c
)
