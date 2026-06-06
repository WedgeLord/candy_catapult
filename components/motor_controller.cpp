#include "motor_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <cmath>

#define TASK_NOTIF_INDEX_RPM 1

void approach_rpm(void * task_args)
{
    int time = 0;
    uint32_t rpm = 0;
    const ledc_channel_config_t *motor_channel_ptr = static_cast<ledc_channel_config_t *>(task_args);
    auto convert_percent_to_duty = [](double percent) -> uint32_t { return (percent > 1 ? 1 : percent) * pow(2, brushed_motor_controller::get_duty_resolution()); };
    while (true)
    {
        printf("waiting: %d == %lu\n", time++, rpm);
        ESP_ERROR_CHECK(
            ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, motor_channel_ptr->channel, convert_percent_to_duty(rpm / 200.0), 100, LEDC_FADE_WAIT_DONE)
        );
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        xTaskNotifyWaitIndexed(TASK_NOTIF_INDEX_RPM, 0, 0, &rpm, portMAX_DELAY);  // finished adjusting
    }
}

std::function<void(int)> create_handle_change_rpm(brushed_motor_controller * controller_ptr)
{
    return [controller_ptr](int rpm) -> void { controller_ptr->change_rpm(rpm); };
}


ledc_timer_bit_t  brushed_motor_controller::get_duty_resolution()
{
    return LEDC_TIMER_8_BIT;
}

brushed_motor_controller::brushed_motor_controller()
: target_rpm { 0 },
  motor_duty_config {
    .speed_mode     = LEDC_LOW_SPEED_MODE,
    .duty_resolution = get_duty_resolution(),
    .timer_num      = LEDC_TIMER_0,
    .freq_hz        = 20000,
    .clk_cfg        = LEDC_AUTO_CLK,
    .deconfigure    = false,
  },
  motor_duty_channel {
    .gpio_num   = 4,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel    = LEDC_CHANNEL_0,
    .intr_type  = LEDC_INTR_MAX,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0,
    .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
    .flags = { .output_invert = false },
    .deconfigure = false,
    }
{
    // motor signal configuration

    ESP_ERROR_CHECK(
        ledc_timer_config(&motor_duty_config)
    );

    ESP_ERROR_CHECK(
        ledc_channel_config(&motor_duty_channel)
    );

    ESP_ERROR_CHECK(
        ledc_fade_func_install(0)
    );

    // tachometer configuration

    if (xTaskCreate(approach_rpm, "motor rpm ctrl", 5000, &motor_duty_config, 3, &rpm_control_task) != pdPASS)
    {
        printf("Failed to create task\n");
        //this->~brushed_motor_controller();
    }
    tach.reset(new hall_sensor_tachometer(create_handle_change_rpm(this)));
}

brushed_motor_controller::~brushed_motor_controller()
{
    printf("Deleting motor controller...\n");
    // tachometer deconfiguration

    vTaskDelete(rpm_control_task);
    

    // motor deconfiguration

    ledc_timer_pause(LEDC_LOW_SPEED_MODE, motor_duty_config.timer_num);

    motor_duty_config.deconfigure = true;
    ESP_ERROR_CHECK(
        ledc_timer_config(&motor_duty_config)
    );

    motor_duty_channel.deconfigure = true;
    ESP_ERROR_CHECK(
        ledc_channel_config(&motor_duty_channel)
    );

    ledc_fade_func_uninstall();
}

// must be interrupt safe
void brushed_motor_controller::change_rpm(int rpm)
{
    configASSERT(xTaskNotifyIndexedFromISR(rpm_control_task, TASK_NOTIF_INDEX_RPM, rpm, eSetValueWithOverwrite, NULL));
    //xTaskGenericNotify(rpm_control_task, TASK_NOTIF_INDEX_RPM, rpm, eSetValueWithOverwrite, NULL);
}

void brushed_motor_controller::set_motor_duty(double percent)
{
    ESP_ERROR_CHECK(
        ledc_set_fade_time_and_start(LEDC_SPEED_MODE_MAX, motor_duty_channel.channel, 100, 1000, LEDC_FADE_WAIT_DONE)
    );
}