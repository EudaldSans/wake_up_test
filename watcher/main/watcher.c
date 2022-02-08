/* Wake up test

    This code sets up a sleeper and a watcher in two diferent ESP32s, 
    the sleeper immediateley goes to sleep, the watcher wakes it up when the boot button is pressed. 

    When the sleeper is awake it notifies the watcher, which will count the time the sleeper slept.

    This is the watcher
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include <sys/time.h>

#include "esp_log.h"


#define WAKE_UP_PIN    18
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<WAKE_UP_PIN))

#define ESP_BUTTON      0
#define WAKE_UP_ACK     4
#define GPIO_INPUT_PIN_SEL  ((1ULL<<WAKE_UP_ACK) | (1ULL<<ESP_BUTTON))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;
static const char* TAG = "[watcher]";

struct timeval time_since_wake_up_signal_sent;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg) {
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            switch(io_num) {
                case ESP_BUTTON: {
                    ESP_LOGI(TAG, "ESP button press");
                    gpio_set_level(WAKE_UP_PIN, 0);
                    gettimeofday(&time_since_wake_up_signal_sent, NULL);
                    gpio_set_level(WAKE_UP_PIN, 1);
                    break;
                }

                case WAKE_UP_ACK: {
                    struct timeval now;
                    gettimeofday(&now, NULL);

                    int sleep_time_ms = (now.tv_sec - time_since_wake_up_signal_sent.tv_sec) * 1000 + (now.tv_usec - time_since_wake_up_signal_sent.tv_usec) / 1000;
                    ESP_LOGI(TAG, "Time spent in deep sleep: %dms", sleep_time_ms);
                    break;
                }
            }
        }
    }
}

void app_main(void) {
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_set_level(WAKE_UP_PIN, 0);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(ESP_BUTTON, gpio_isr_handler, (void*) ESP_BUTTON);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(WAKE_UP_ACK, gpio_isr_handler, (void*) WAKE_UP_ACK);

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}
