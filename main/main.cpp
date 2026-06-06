//#include "../components/hall_sensor_tachometer.h"
#include "../components/motor_controller.h"

#include "freertos/FreeRTOS.h"

#include <stdio.h>

extern "C" void app_main(void)
{
    int time = 0;
    //hall_sensor_tachometer tach;
    struct timeval current_measured_time;
    //while (true)
    while (false)
    {
        gettimeofday(&current_measured_time, NULL);
        printf("waiting: %d == %llu.%lu\n", time++, current_measured_time.tv_sec, current_measured_time.tv_usec);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    /**/
    }
    brushed_motor_controller motor;
    motor.change_rpm(50);
    vTaskDelay(100'000 / portTICK_PERIOD_MS);
    motor.change_rpm(0);
    vTaskDelay(1'000 / portTICK_PERIOD_MS);
}
