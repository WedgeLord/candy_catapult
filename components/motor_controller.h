#include "hall_sensor_tachometer.h"
#include "driver/ledc.h"
#include <memory>


class brushed_motor_controller
{
private:
    TaskHandle_t rpm_control_task;
    std::unique_ptr<hall_sensor_tachometer> tach;
    int target_rpm;
    ledc_timer_config_t motor_duty_config;
    ledc_channel_config_t motor_duty_channel;

public:
    brushed_motor_controller();
    ~brushed_motor_controller();

    void update_real_rpm(uint32_t rpm);
    void update_target_rpm(uint32_t rpm);

    static ledc_timer_bit_t  get_duty_resolution();

private:
    void set_motor_duty(double percent);

};
