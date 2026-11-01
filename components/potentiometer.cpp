#include "potentiometer.h"
#include <cmath>

potentiometer::potentiometer(pot_callback cb)
{
    adc_continuous_handle_cfg_t pot_cfg = {
        .max_store_buf_size = 4,
        .conv_frame_size = 4, // = SOC_ADC_DIGI_DATA_BYTES_PER_CONV, // apparently must be a multiple of 4, not just 2, or 'hal' asserts fail
        .flags = { .flush_pool = true }
    };

    ESP_ERROR_CHECK(
        adc_continuous_new_handle(&pot_cfg, &adc_reader)
    );

    adc_unit_t unit;
    adc_channel_t channel;
    adc_continuous_io_to_channel(5, &unit, &channel);
    adc_digi_pattern_config_t pattern[1] = {
        {
        .atten = ADC_ATTEN_DB_12, // 12db attenuation reduces input voltage (5V) by about 4x due to the max voltage reading of 1.1V
        .channel = channel,
        .unit = unit,
        .bit_width = ADC_BITWIDTH_12,
        }
    };
    adc_continuous_config_t pot_reader = {
        .pattern_num = 1,
        .adc_pattern = pattern,
        .sample_freq_hz = 1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };
    ESP_ERROR_CHECK(
        adc_continuous_config(adc_reader, &pot_reader)
    );
    struct adc_callback_params
    {
        pot_callback func;
        const adc_continuous_handle_t adc;
    };
    adc_continuous_evt_cbs_t callbacks_with_params = {
        .on_conv_done = [](adc_continuous_handle_t, const adc_continuous_evt_data_t *data, void *ctx) -> bool { 
            adc_callback_params *params = (adc_callback_params *)ctx;
            if (params == NULL) return false;
            data_to_level(params->adc, data)(params->func);
            return false; 
        },
        .on_pool_ovf = NULL,
    };
    adc_continuous_evt_cbs_t callbacks = {
        .on_conv_done = [](adc_continuous_handle_t, const adc_continuous_evt_data_t *data, void *ctx) -> bool { 
            if (ctx == NULL) return false;
            pot_callback callback = *static_cast<pot_callback *>(ctx);
            callback(*reinterpret_cast<uint16_t *>(data->conv_frame_buffer));
            return false;
        },
        .on_pool_ovf = NULL,
    };

    static adc_callback_params params = { 
        .func = cb,
        .adc = adc_reader, 
    };
    ESP_ERROR_CHECK(
        adc_continuous_register_event_callbacks(adc_reader, &callbacks_with_params, &params)
        //adc_continuous_register_event_callbacks(adc_reader, &callbacks, &callback)
    );
    ESP_ERROR_CHECK(
        adc_continuous_start(adc_reader)
    );
}

potentiometer::~potentiometer()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        adc_continuous_stop(adc_reader)
    );
    adc_continuous_deinit(adc_reader);
}

std::function<void(pot_callback)> potentiometer::data_to_level(const adc_continuous_handle_t adc, const adc_continuous_evt_data_t *data)
{
    static volatile double last_level = 0.0;
    uint32_t count = 0;
    adc_continuous_data_t level[1];
    if (adc != NULL)
    {
        adc_continuous_parse_data(adc, data->conv_frame_buffer, sizeof(level[0]), level, &count);
        if (count > 0 && level->valid)
        {
            double new_level = level->raw_data / pow(2, 12);
            double diff = last_level - new_level;
            if (diff > 0.02 || diff < -0.02)
            {
                last_level = new_level;
                return [new_level](pot_callback callback) -> void {
                    callback(new_level);
                };
            }
        }
    }
    return [](pot_callback){};
}
