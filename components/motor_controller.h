#include "hall_sensor_tachometer.h"
#include <memory>


class brushed_motor_controller
{
private:
    TaskHandle_t rpm_control_task;
    std::unique_ptr<hall_sensor_tachometer> tach;
    int target_rpm;


public:
    brushed_motor_controller();
    ~brushed_motor_controller();

    void change_rpm(int rpm);

private:

};
