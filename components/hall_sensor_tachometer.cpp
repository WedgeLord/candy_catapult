#include "hall_sensor_tachometer.h"

#include "driver/gpio.h"

#define ROLLING_BUFFER_SIZE 5

//#include <memory>

void call_update_on_tachometer(void * isr_tach)
{
    if (isr_tach == nullptr)
    {
        //printf("tachometer is null...\n");
        return;
    }

    hall_sensor_tachometer* tach = static_cast<hall_sensor_tachometer*>(isr_tach);
    tach->update();
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
  onUpdate { callback }
{
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
        gpio_isr_handler_add(GPIO_NUM_2, call_update_on_tachometer, this)
    );
    for (int p : rolling_buffer) { printf("%d | ", p); }
}

hall_sensor_tachometer::~hall_sensor_tachometer()
{
    gpio_isr_handler_remove(GPIO_NUM_2);
}

void hall_sensor_tachometer::update()
{
    static struct timeval last_measured_time = { .tv_sec = 0, .tv_usec = 0 };

    struct timeval current_measured_time;
    gettimeofday(&current_measured_time, NULL);

    suseconds_t us_delta = current_measured_time.tv_usec - last_measured_time.tv_usec;

    if (us_delta > 0 && last_measured_time.tv_usec != 0)
    {
        avg_period -= rolling_buffer[index] / ROLLING_BUFFER_SIZE;
        rolling_buffer[index] = us_delta;
        avg_period += rolling_buffer[index] / ROLLING_BUFFER_SIZE;
        index += 1;
        index %= ROLLING_BUFFER_SIZE;

        if (onUpdate != NULL)
        {
            onUpdate(get_freq());
        }
    }
    last_measured_time = current_measured_time;
}

double hall_sensor_tachometer::get_freq()
{
    if (avg_period <= 0)
    {
        return -1.0;
    }
    return 60 / ( avg_period / 1'000'000 );
}