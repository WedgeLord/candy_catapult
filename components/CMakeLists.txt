idf_component_register(SRCS "motor_controller.cpp" "hall_sensor_tachometer.cpp"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES
                        esp_driver_gpio
)
