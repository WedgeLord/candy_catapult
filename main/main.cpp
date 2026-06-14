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
    motor.update_target_rpm(180);
    vTaskDelay(20'000 / portTICK_PERIOD_MS);
    motor.update_target_rpm(75);
    vTaskDelay(portMAX_DELAY);
    vTaskDelay(100'000 / portTICK_PERIOD_MS);
    motor.update_target_rpm(0);
    vTaskDelay(3'000 / portTICK_PERIOD_MS);
}
