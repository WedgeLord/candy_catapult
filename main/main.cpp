#include "../components/hall_sensor_tachometer.h"

#include "freertos/FreeRTOS.h"

#include <stdio.h>

extern "C" void app_main(void)
{
    int time = 0;
    hall_sensor_tachometer tach;
    while (true)
    {
        printf("waiting: %d == %.2f\n", time++, tach.get_freq());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
