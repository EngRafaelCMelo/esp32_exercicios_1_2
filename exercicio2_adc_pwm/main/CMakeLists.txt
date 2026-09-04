idf_component_register(
    SRCS "exercicio2_adc_pwm.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_ledc esp_adc
)