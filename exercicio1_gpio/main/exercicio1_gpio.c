#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Bibliotecas ESP-IDF para GPIO, temporização e logs
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED1_PIN       GPIO_NUM_21
#define LED2_PIN       GPIO_NUM_22
#define LED3_PIN       GPIO_NUM_23
#define BUTTON1_PIN    GPIO_NUM_19
#define BUTTON2_PIN    GPIO_NUM_4

#define BUTTON_PRESSED_LEVEL  0
#define DEBOUNCE_TIME_MS      50
#define BUTTON_SCAN_TIME_MS   10
#define LED3_HALF_PERIOD_MS   500
#define MILLISECONDS_TO_MICROSECONDS(value) ((int64_t)(value) * 1000LL)

static const char *TAG_BUTTON = "Status do botão";
static const char *TAG_LED = "Status do LED";

typedef struct {
    gpio_num_t button_pin;
    gpio_num_t led_pin;
    int button_number;
    bool last_sample_pressed;
    bool stable_pressed;
    int64_t last_sample_change_us;
    uint32_t led_level;
} button_control_t;

static void configure_gpio(void)
{
    const gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED1_PIN) |
                        (1ULL << LED2_PIN) |
                        (1ULL << LED3_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&led_config));

    const gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON1_PIN) |
                        (1ULL << BUTTON2_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_config));

    ESP_ERROR_CHECK(gpio_set_level(LED1_PIN, 0));
    ESP_ERROR_CHECK(gpio_set_level(LED2_PIN, 0));
    ESP_ERROR_CHECK(gpio_set_level(LED3_PIN, 0));
}

static void initialize_button_state(button_control_t *button)
{
    const bool pressed =
        gpio_get_level(button->button_pin) == BUTTON_PRESSED_LEVEL;

    button->last_sample_pressed = pressed;
    button->stable_pressed = pressed;
    button->last_sample_change_us = esp_timer_get_time();
    button->led_level = 0;
}

static void update_button(button_control_t *button, int64_t now_us)
{
    const bool sample_pressed =
        gpio_get_level(button->button_pin) == BUTTON_PRESSED_LEVEL;

    if (sample_pressed != button->last_sample_pressed) {
        button->last_sample_pressed = sample_pressed;
        button->last_sample_change_us = now_us;
    }

    const bool sample_is_stable =
        (now_us - button->last_sample_change_us) >=
        MILLISECONDS_TO_MICROSECONDS(DEBOUNCE_TIME_MS);

    if (sample_is_stable && sample_pressed != button->stable_pressed) {
        button->stable_pressed = sample_pressed;

        if (sample_pressed) {
            button->led_level = !button->led_level;
            ESP_ERROR_CHECK(gpio_set_level(button->led_pin, button->led_level));

            ESP_LOGI(TAG_BUTTON, "Botão [%d] pressionado.",
                     button->button_number);
            ESP_LOGI(TAG_LED, "LED%d %s", button->button_number,
                     button->led_level ? "aceso" : "apagado");
        } else {
            ESP_LOGI(TAG_BUTTON, "Botão [%d] liberado.",
                     button->button_number);
            ESP_LOGI(TAG_LED, "LED%d permanece %s", button->button_number,
                     button->led_level ? "aceso" : "apagado");
        }
    }
}

static void delay_ms(uint32_t milliseconds)
{
    usleep(milliseconds * 1000U);
}

void app_main(void)
{
    configure_gpio();

    button_control_t button1 = {
        .button_pin = BUTTON1_PIN,
        .led_pin = LED1_PIN,
        .button_number = 1,
    };
    button_control_t button2 = {
        .button_pin = BUTTON2_PIN,
        .led_pin = LED2_PIN,
        .button_number = 2,
    };

    initialize_button_state(&button1);
    initialize_button_state(&button2);

    uint32_t led3_level = 1;
    ESP_ERROR_CHECK(gpio_set_level(LED3_PIN, led3_level));
    ESP_LOGI(TAG_LED, "LED3 aceso");
    int64_t last_led3_change_us = esp_timer_get_time();

    ESP_LOGI(TAG_LED, "Exercício 1 iniciado");

    while (true) {
        const int64_t now_us = esp_timer_get_time();

        update_button(&button1, now_us);
        update_button(&button2, now_us);

        if ((now_us - last_led3_change_us) >=
            MILLISECONDS_TO_MICROSECONDS(LED3_HALF_PERIOD_MS)) {
            led3_level = !led3_level;
            ESP_ERROR_CHECK(gpio_set_level(LED3_PIN, led3_level));
            ESP_LOGI(TAG_LED, "LED3 %s", led3_level ? "aceso" : "apagado");
            last_led3_change_us = now_us;
        }

        delay_ms(BUTTON_SCAN_TIME_MS);
    }
}
