#include "hall_sensor_tachometer.h"

#include "driver/gpio.h"
#include "driver/gpio_filter.h"

#define ROLLING_BUFFER_SIZE 3


void add_time_to_queue(void * isr_queue)
{
    QueueHandle_t queue = *static_cast<QueueHandle_t *>(isr_queue);
    struct timeval current_measured_time;
    //gettimeofday(&current_measured_time, NULL);
    xQueueOverwriteFromISR(queue, &current_measured_time, NULL);
}

hall_sensor_tachometer::hall_sensor_tachometer()
{
    hall_sensor_tachometer(NULL);
}

hall_sensor_tachometer::hall_sensor_tachometer(std::function<void(int)> callback)
: count { 0 },
  avg_period { 0 },
  rolling_buffer ( ROLLING_BUFFER_SIZE, 0 ),
  index { 0 },
  latest_measured_time_handle { xQueueCreate(1, sizeof(struct timeval)) },
  onUpdate { callback }
{
    xTaskCreate([](void *tach) -> void { static_cast<hall_sensor_tachometer *>(tach)->update(); }, "tach_loop", 1000, this, 3, &update_task_handle);

    gpio_config_t  tach_gpio_config = {
        .pin_bit_mask = (1<<2),  // pin #2
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(
        gpio_config(&tach_gpio_config)
    );
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_install_isr_service(ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_LEVEL3)
    );
    ESP_ERROR_CHECK(
        gpio_isr_handler_add(GPIO_NUM_2, add_time_to_queue, &latest_measured_time_handle)
    );

    gpio_pin_glitch_filter_config_t glitch_filter_cfg = {
        .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
        .gpio_num = GPIO_NUM_2,
    };
    gpio_glitch_filter_handle_t glitch_filter;
    ESP_ERROR_CHECK(
        gpio_new_pin_glitch_filter(&glitch_filter_cfg, &glitch_filter)
    );
    ESP_ERROR_CHECK(
        gpio_glitch_filter_enable(glitch_filter)
    );
}

hall_sensor_tachometer::~hall_sensor_tachometer()
{
    ESP_ERROR_CHECK(
        gpio_isr_handler_remove(GPIO_NUM_2)
    );

    vTaskDelete(update_task_handle);
}

void hall_sensor_tachometer::update()
{
    struct timeval last_measured_time = { 0, 0 };
    while (true)
    {
        for (int p : rolling_buffer) { printf("%d | ", p); }; puts("");

        struct timeval current_measured_time;
        if (xQueueReceive(latest_measured_time_handle, &current_measured_time, portMAX_DELAY) != pdTRUE)
        {
            printf("queue grab failed idk\n");
        }
        gettimeofday(&current_measured_time, NULL);  // can't call this function in ISR unfortunately

        suseconds_t us_delta = current_measured_time.tv_usec - last_measured_time.tv_usec;
        us_delta += (current_measured_time.tv_sec - last_measured_time.tv_sec) * 1E+6;

        if (us_delta > (60E+6 / 200) * 0.5)
        {// smallest reasonable period is 1min / 200rpm, as 200rpm is motor max speed, and period is halved again for unexpected cases up to 400rpm
            avg_period -= rolling_buffer[index] / ROLLING_BUFFER_SIZE;
            rolling_buffer[index] = us_delta;
            avg_period += rolling_buffer[index] / ROLLING_BUFFER_SIZE;
            index += 1;
            index %= ROLLING_BUFFER_SIZE;

            if (onUpdate != NULL)
            {
                onUpdate(get_freq());
            }
            last_measured_time = current_measured_time;
        }
    }
}

double hall_sensor_tachometer::get_freq()
{
    if (avg_period <= 0)
    {
        return 0;
    }
    return 60 / ( avg_period / 1E+6 );
}