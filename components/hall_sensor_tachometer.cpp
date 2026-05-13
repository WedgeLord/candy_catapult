#include "hall_sensor_tachometer.h"

#include "freertos/FreeRTOS.h"

hall_sensor_tachometer::hall_sensor_tachometer()
: count { 0 }
{
}

void hall_sensor_tachometer::update()
{
    count += 1;
    printf("%d\n", count);
}