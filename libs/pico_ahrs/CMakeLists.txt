add_library(pico_ahrs STATIC
    src/Adafruit_AHRS_Madgwick.cpp
    src/Adafruit_AHRS_Mahony.cpp
    src/Adafruit_AHRS_NXPFusion.cpp
    src/MadgwickAHRS.cpp
)

target_include_directories(pico_ahrs PUBLIC include)

target_link_libraries(pico_ahrs PUBLIC pico_stdlib)
