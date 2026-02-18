add_library(pico_imu_sensors STATIC
    src/BaseIMU.cpp
    src/Accelerometer.cpp
    src/Gyroscope.cpp
    src/Compass.cpp
)

target_include_directories(pico_imu_sensors PUBLIC include)

target_link_libraries(pico_imu_sensors PUBLIC
    pico_stdlib
    hardware_i2c
)
