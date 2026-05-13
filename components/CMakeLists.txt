idf_component_register(SRCS "hall_sensor_tachometer.cpp"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES
                        esp_driver_gpio
)
