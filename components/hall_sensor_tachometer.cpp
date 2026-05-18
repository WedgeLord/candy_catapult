#include "hall_sensor_tachometer.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

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
: count { 0 }
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
    ESP_ERROR_CHECK(
        gpio_install_isr_service(ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_LEVEL3)
    );
    ESP_ERROR_CHECK(
        //gpio_isr_register(call_update_on_tachometer, this, (ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_LEVEL3), NULL)
        gpio_isr_handler_add(GPIO_NUM_2, call_update_on_tachometer, this)
    );
    esp_intr_dump(stdout);
}

void hall_sensor_tachometer::update()
{
    count += 1;
    //printf("%d\n", count);
}