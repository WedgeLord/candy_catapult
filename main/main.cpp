//#include "../components/hall_sensor_tachometer.h"
#include "../components/motor_controller.h"
#include "../components/potentiometer.h"

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
    }
    brushed_motor_controller motor;
    potentiometer motor_control([&motor](double level) -> void {
        motor.update_target_rpm(level * 550);
    });
    //vTaskDelay(20'000 / portTICK_PERIOD_MS);
    //motor.update_target_rpm(0);
    vTaskDelay(portMAX_DELAY);
    /*
    QueueHandle_t q = xQueueCreate(1, sizeof(double));
    potentiometer pot([q](double level) -> void { xQueueOverwriteFromISR(q, &level, NULL); return false; });
    while (true)
    {
        double v = 0.0;
        xQueueReceive(q, &v, portMAX_DELAY);
        printf("level: %lf\n", v);
    }
    //pot.~potentiometer();
    */
}
