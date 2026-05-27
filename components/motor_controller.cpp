#include "motor_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>

#define TASK_NOTIF_INDEX_RPM 1

void approach_rpm(void *)
{
    int time = 0;
    uint32_t rpm = 0;
    while (true)
    {
        printf("waiting: %d == %lu\n", time++, rpm);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        xTaskNotifyWaitIndexed(TASK_NOTIF_INDEX_RPM, 0, 0, &rpm, portMAX_DELAY);  // finished adjusting
    }
}

std::function<void(int)> create_handle_change_rpm(brushed_motor_controller * controller_ptr)
{
    return [controller_ptr](int rpm) -> void { controller_ptr->change_rpm(rpm); };
}


brushed_motor_controller::brushed_motor_controller()
: target_rpm { 0 }
{
    if (xTaskCreate(approach_rpm, "motor rpm ctrl", 5000, NULL, 3, &rpm_control_task) != pdPASS)
    {
        printf("Failed to create task\n");
        //this->~brushed_motor_controller();
    }
    tach.reset(new hall_sensor_tachometer(create_handle_change_rpm(this)));
}

brushed_motor_controller::~brushed_motor_controller()
{
    vTaskDelete(rpm_control_task);
}

// must be interrupt safe
void brushed_motor_controller::change_rpm(int rpm)
{
    configASSERT(xTaskNotifyIndexedFromISR(rpm_control_task, TASK_NOTIF_INDEX_RPM, rpm, eSetValueWithOverwrite, NULL));
    //xTaskGenericNotify(rpm_control_task, TASK_NOTIF_INDEX_RPM, rpm, eSetValueWithOverwrite, NULL);
}
