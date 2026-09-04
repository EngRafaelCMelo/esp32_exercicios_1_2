#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#define ADC_UNIT_USED          ADC_UNIT_2
#define ADC_CHANNEL_USED       ADC_CHANNEL_0  // ADC2_CH0 = GPIO4 (D4) no ESP32
#define ADC_ATTENUATION        ADC_ATTEN_DB_12
#define ADC_BIT_WIDTH          ADC_BITWIDTH_12
#define ADC_MAX_RAW            4095
#define ADC_SAMPLE_COUNT       32

#define LED_PWM_PIN            GPIO_NUM_21
#define LEDC_MODE_USED         LEDC_LOW_SPEED_MODE
#define LEDC_TIMER_USED        LEDC_TIMER_0
#define LEDC_CHANNEL_USED      LEDC_CHANNEL_0
#define LEDC_FREQUENCY_HZ      5000
#define LEDC_DUTY_RESOLUTION   LEDC_TIMER_12_BIT
#define LEDC_MAX_DUTY          4095U

#define UPDATE_PERIOD_MS       200

static const char *TAG = "ADC e PWM";

static adc_oneshot_unit_handle_t configure_adc(void)
{
    adc_oneshot_unit_handle_t adc_handle;

    const adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_USED,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BIT_WIDTH,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        ADC_CHANNEL_USED,
        &channel_config
    ));

    return adc_handle;
}

static adc_cali_handle_t configure_adc_calibration(void)
{
    adc_cali_handle_t calibration_handle = NULL;

    const adc_cali_line_fitting_config_t calibration_config = {
        .unit_id = ADC_UNIT_USED,
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BIT_WIDTH,
        .default_vref = 1100,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(
        &calibration_config,
        &calibration_handle
    ));

    return calibration_handle;
}

static void configure_pwm(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_MODE_USED,
        .duty_resolution = LEDC_DUTY_RESOLUTION,
        .timer_num = LEDC_TIMER_USED,
        .freq_hz = LEDC_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    const ledc_channel_config_t channel_config = {
        .gpio_num = LED_PWM_PIN,
        .speed_mode = LEDC_MODE_USED,
        .channel = LEDC_CHANNEL_USED,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_USED,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

static int read_adc_average(adc_oneshot_unit_handle_t adc_handle)
{
    int64_t sample_sum = 0;

    for (int sample = 0; sample < ADC_SAMPLE_COUNT; sample++) {
        int adc_raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(
            adc_handle,
            ADC_CHANNEL_USED,
            &adc_raw
        ));
        sample_sum += adc_raw;
    }

    return (int)(sample_sum / ADC_SAMPLE_COUNT);
}

static void set_led_duty(uint32_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(
        LEDC_MODE_USED,
        LEDC_CHANNEL_USED,
        duty
    ));
    ESP_ERROR_CHECK(ledc_update_duty(
        LEDC_MODE_USED,
        LEDC_CHANNEL_USED
    ));
}

static void delay_ms(uint32_t milliseconds)
{
    usleep(milliseconds * 1000U);
}

static void update_adc_pwm(
    adc_oneshot_unit_handle_t adc_handle,
    adc_cali_handle_t calibration_handle)
{
    const int adc_raw = read_adc_average(adc_handle);
    int voltage_mv = 0;

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
        calibration_handle,
        adc_raw,
        &voltage_mv
    ));

    const uint32_t duty =
        ((uint32_t)adc_raw * LEDC_MAX_DUTY) / ADC_MAX_RAW;
    const uint32_t duty_percent_x10 =
        (duty * 1000U) / LEDC_MAX_DUTY;

    set_led_duty(duty);

    ESP_LOGI(
        TAG,
        "ADC: %d | Tensão: %d mV | Duty: %" PRIu32 "/%u (%" PRIu32 ".%" PRIu32 "%%)",
        adc_raw,
        voltage_mv,
        duty,
        LEDC_MAX_DUTY,
        duty_percent_x10 / 10U,
        duty_percent_x10 % 10U
    );
}

void app_main(void)
{
    const adc_oneshot_unit_handle_t adc_handle = configure_adc();
    const adc_cali_handle_t calibration_handle = configure_adc_calibration();
    configure_pwm();

    ESP_LOGI(TAG, "Exercício 2 iniciado");

    while (true) {
        update_adc_pwm(adc_handle, calibration_handle);
        delay_ms(UPDATE_PERIOD_MS);
    }
}
