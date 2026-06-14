#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <deque>
#include <functional>
#include <sys/time.h>


class hall_sensor_tachometer
{
public:
    int count;
    double avg_period;
private:
    std::deque<suseconds_t> rolling_buffer;
    int index;
    QueueHandle_t latest_measured_time_handle;
    TaskHandle_t update_task_handle;
    std::function<void(int)> onUpdate;

public:
    hall_sensor_tachometer();
    explicit hall_sensor_tachometer(std::function<void(int)>);
    ~hall_sensor_tachometer();

    void update();

    double get_freq();  // frequency per minute
};
