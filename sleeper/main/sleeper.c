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

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"


#define WAKE_ACK            18
#define WAKE_UP_PIN         33
#define WAKE_UP_LEVEL       1

#define DEEP_SLEEP

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<WAKE_ACK))

static RTC_DATA_ATTR gpio_config_t io_conf;

static const char* TAG = "[sleeper]";


void go_to_sleep(){
    ESP_LOGI(TAG, "Enabling EXT1 wakeup on pin GPIO %d", WAKE_UP_PIN);
    // esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_ext0_wakeup(WAKE_UP_PIN, WAKE_UP_LEVEL);

    ESP_LOGI(TAG, "Going to sleep. ZzZ... zZz...\n");

#ifdef DEEP_SLEEP
    esp_deep_sleep_start();
#else
    ets_delay_us(5000);
    esp_light_sleep_start();
#endif

}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO); // At this point we can enable logging again for easier debugging  

    while(1) {
        switch (esp_sleep_get_wakeup_cause()) {    
            case ESP_SLEEP_WAKEUP_EXT0:
            case ESP_SLEEP_WAKEUP_EXT1:

#ifdef DEEP_SLEEP
                gpio_config(&io_conf);
#endif
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

                gpio_config(&io_conf);

                ESP_LOGI(TAG, "Not a deep sleep reset");
                break;
        }

        ESP_LOGI(TAG, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

        go_to_sleep();
    }
}
