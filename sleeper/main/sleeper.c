/* Wake up test

    This code sets up a sleeper and a watcher in two diferent ESP32s, 
    the sleeper immediateley goes to sleep, the watcher wakes it up when the boot button is pressed. 

    When the sleeper is awake it notifies the watcher, which will count the time the sleeper slept.

    This is the sleeper
*/
#include "esp_sleep.h"
#include "soc/rtc.h"

#include "esp_log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_log.h"


#define WAKE_ACK    18
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<WAKE_ACK))

static RTC_DATA_ATTR gpio_config_t io_conf;

static const char* TAG = "[sleeper]";

/*
    Improvements to wake up time:
        - Serial flasher config
            + flash SPI mode -> QUIO (improvement of ~5ms)
            + flash SPI speed -> 80MHz (improvement of ~5ms)
        
        - Component config 
            + default log verbosity -> no output (improvement of ~120 ms)

        - Bootloader config
            + bootloader log verbosity -> warning (improvement of ~100ms)
            + bootloader optimisation level -> optimize for performance (-O2) (improvement of ~ 5ms)
            + skip image validation when exiting deep sleep (improvement of ~55ms)

    When all applied wake up time goes from 344ms to 44ms
*/

void go_to_sleep(){
    const int ext_wakeup_pin_1 = 2;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

    ESP_LOGI(TAG, "Enabling EXT1 wakeup on pin GPIO %d", ext_wakeup_pin_1);
    // esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_ext0_wakeup(2, 1);

    ESP_LOGI(TAG, "Entering deep sleep");

    esp_deep_sleep_start();
}

void app_main(void) {
    switch (esp_sleep_get_wakeup_cause()) {       
        case ESP_SLEEP_WAKEUP_EXT0:º
        case ESP_SLEEP_WAKEUP_EXT1:
            gpio_config(&io_conf);

            gpio_set_level(WAKE_ACK, 1);
            gpio_set_level(WAKE_ACK, 0);
            ESP_LOGI(TAG, "Woke up from sleep");

            break;

        case ESP_SLEEP_WAKEUP_TIMER: {
            ESP_LOGI(TAG, "Woke up from timer.");
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
            io_conf.pull_down_en = 1;
            io_conf.pull_up_en = 0;

            ESP_LOGI(TAG, "Not a deep sleep reset");
            break;
    }

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    go_to_sleep();
}
