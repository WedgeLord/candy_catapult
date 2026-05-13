#include "../components/hall_sensor_tachometer.h"

#include "freertos/FreeRTOS.h"

#include <stdio.h>

extern "C" void app_main(void)
{
    hall_sensor_tachometer tach;
    while (true)
    {
        tach.update();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
