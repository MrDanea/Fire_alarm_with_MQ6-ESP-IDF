#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "buzzer.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/timers.h"
#include "freertos/portmacro.h"
#include "driver/rtc_io.h"
#include "Task.h"
#include "main.h"

#define DEBOUNCE_TIME_MS 50  // Thời gian chống chattering (ms)
#define BTN_GPIO 2
#define WARNING_BTN_GPIO 4
static TimerHandle_t debounceTimer = NULL;
volatile bool warning_task = false;
volatile bool time_stop = false;
volatile bool button_stop = false;
volatile bool buzzer_enabled = false;



void debounce_timer_callback(TimerHandle_t xTimer)
{
    // stop_buzzer_task = true;
    // warning_task = false;
     // Xử lý khi đã đủ thời gian chống chattering - Không làm gì c
}


void button_task(void *pvParameter)
{
    esp_rom_gpio_pad_select_gpio(BTN_GPIO);
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);

    while (true) {
        if (gpio_get_level(BTN_GPIO) == 0) {
            // Nút được nhấn, bắt đầu đếm thời gian chống chattering

            if (debounceTimer == NULL) {
                debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, NULL, debounce_timer_callback);
                xTimerStart(debounceTimer, portMAX_DELAY);
            }
            // Thêm một chút độ trễ để giảm tải CPU
            vTaskDelay(pdMS_TO_TICKS(10));

            // buzzer_enabled = false;
            button_stop = true;
            warning_task = false;
            WARNING_DELAY = false;
        }
        else {
            // Nút được thả, tắt đèn và dừng timer
            
            if (debounceTimer != NULL) {
                xTimerReset(debounceTimer, portMAX_DELAY);
            }

            // Thêm một chút độ trễ để giảm tải CPU
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        
    }
    
    vTaskDelete(NULL);
}

void warningbtn_task(void *pvParameter){
    esp_rom_gpio_pad_select_gpio(WARNING_BTN_GPIO);
    gpio_set_direction(WARNING_BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(WARNING_BTN_GPIO, GPIO_PULLUP_ONLY);

    while (true) {
        if (gpio_get_level(WARNING_BTN_GPIO) == 0) {
            // Nút được nhấn, bắt đầu đếm thời gian chống chattering

            if (debounceTimer == NULL) {
                debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, NULL, debounce_timer_callback);
                xTimerStart(debounceTimer, portMAX_DELAY);
            }
            // Thêm một chút độ trễ để giảm tải CPU
            vTaskDelay(pdMS_TO_TICKS(10));

            // buzzer_enabled = true;
            warning_task = true;
            button_stop = false;
            time_stop = false;
            WARNING_DELAY = true;
        }
        else {
            
            
            // Nút được thả, tắt đèn và dừng timer
            if (debounceTimer != NULL) {
                xTimerReset(debounceTimer, portMAX_DELAY);
            }

            // Thêm một chút độ trễ để giảm tải CPU
            vTaskDelay(pdMS_TO_TICKS(10));
        }

    }

    vTaskDelete(NULL);
}

void buzzer_task(void *pvParameters) {
    // Nội dung của tác vụ
    while (true) {
        if(!STOP(time_stop, button_stop, warning_task)){
            buzzer(NOTE_D1, 7168, 1, 1, 5);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

bool STOP(bool time, bool button, bool warning){
    while(true){
        if(warning == false) return true;
        else {
            if(time == true || button == true) return true;
            else return false;
        } 
    }
    vTaskDelete(NULL);   
}
