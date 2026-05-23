#include "freertos/FreeRTOS.h"

#include <deque>


class hall_sensor_tachometer
{
public:
    int count;
    double avg_period;
private:
    std::deque<TickType_t> rolling_buffer;
    int index;

public:
    hall_sensor_tachometer();

    void update();

    double get_freq();  // frequency per minute
};
