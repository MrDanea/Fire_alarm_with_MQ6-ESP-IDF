#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "light_sleep_example.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "main.h"
#include "LED.h"
#include "am2302_rmt.h"
#include "freertos/timers.h"
#include "freertos/portmacro.h"
#include "driver/rtc_io.h"
#include "buzzer.h"
#include "Task.h"

#define MQ6_PIN_ANALOG 0
#define AM2302_GPIO 1
// #define LED_GPIO GPIO_NUM_4
// #define BTN_GPIO GPIO_NUM_3
/*Tạo độ trễ duy trì chế độ đánh thức của esp32c3 */
volatile bool WARNING_DELAY = false;
volatile float WARNING_POINT = 0;

void InitAM2302Sensor(am2302_handle_t *sensor);
void InitGpioOutput(int GPIO_PIN);
void light_sleep_task(void *args);
void MQ6_setup();
void InitGpioInput(gpio_num_t GPIO_PIN);
float Get_mVolt(gpio_num_t GPIO_PIN);
float Calculate_Rs(float Vo);
void example_wait_gpio_inactive();

// static const char *TAG_Gas = "Nguong khi ga: ";
// static const char *TAG_Temp = "Nhiet do, do am: ";
am2302_handle_t sensor = NULL;
/// 
float temperature;
float humidity;
// tinh toan khi gas
float Referance_V = 3300.0;
float RL = 1.0;
float Ro = 10.0;
float mVolt = 0.0;
const float Ro_clean_air_factor = 10.0;

void app_main(void)
{
    InitGpioInput(MQ6_PIN_ANALOG); // Cau hinh chan nhan MQ6
//  Cau hinhchan ra cho 74hc595 dieu khien led
    MQ6_setup();
    buzzer_init(5);
    InitGpioOutput(stcp_Pin); 
    InitGpioOutput(shcp_Pin);
    InitGpioOutput(ds_Pin);
    xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(warningbtn_task, "warningbtn_task", 2048, NULL, 5, NULL);
// Cau hinh chan vao cho dht11
    InitAM2302Sensor(&sensor);
// Dat tre doi thiet bij hoat dong on dinh
    vTaskDelay(pdMS_TO_TICKS(15000));
// Khoi tao chan danh thuc
    example_register_gpio_wakeup();
    //chckbtn_register_gpio_wakeup();
    
//Dua vao giac ngu ngan thu
    xTaskCreate(light_sleep_task, "light_sleep_task", 4096, NULL, 6, NULL);
}


void light_sleep_task(void *args)
{

    while (true) {
        printf("Entering light sleep\n");
        /* To make sure the complete line is printed before entering sleep mode,
         * need to wait until UART TX FIFO is empty:
         */
        uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);

        /* Get timestamp before entering sleep */
        int64_t t_before_us = esp_timer_get_time();

        /* Enter sleep mode */
        esp_light_sleep_start();

        /* Get timestamp after waking up from sleep */
        int64_t t_after_us = esp_timer_get_time();

        /* Determine wake up reason */
        const char* wakeup_reason;
        switch (esp_sleep_get_wakeup_cause()) {
            case ESP_SLEEP_WAKEUP_TIMER:
                wakeup_reason = "timer";
                break;
            case ESP_SLEEP_WAKEUP_GPIO:
                wakeup_reason = "pin";
                break;
            case ESP_SLEEP_WAKEUP_UART:
                wakeup_reason = "uart";
                /* Hang-up for a while to switch and execuse the uart task
                 * Otherwise the chip may fall sleep again before running uart task */
                vTaskDelay(1);
                break;
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
            case ESP_SLEEP_WAKEUP_TOUCHPAD:
                wakeup_reason = "touch";
                break;
#endif
            default:
                wakeup_reason = "other";
                break;
        }
#if CONFIG_NEWLIB_NANO_FORMAT
        /* printf in newlib-nano does not support %ll format, causing example test fail */
        printf("Returned from light sleep, reason: %s, t=%d ms, slept for %d ms\n",
                wakeup_reason, (int) (t_after_us / 1000), (int) ((t_after_us - t_before_us) / 1000));
#else
        printf("Returned from light sleep, reason: %s, t=%lld ms, slept for %lld ms\n",
                wakeup_reason, t_after_us / 1000, (t_after_us - t_before_us) / 1000);
#endif
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
            /* Waiting for the gpio inactive, or the chip will continously trigger wakeup*/

            // uint64_t gpio_status = esp_sleep_get_gpio_wakeup_status();
            // printf("%llu\n", gpio_status);
            example_wait_gpio_inactive();
        }
        
        vTaskDelay(pdMS_TO_TICKS(20000));
        while(WARNING_DELAY){
            vTaskDelay(pdMS_TO_TICKS(20000));
        }
        
    }
    vTaskDelete(NULL);
}

void MQ6_setup() {

    for (int i = 0; i < 2000; i++) {
        mVolt += Get_mVolt(MQ6_PIN_ANALOG);
    }
    mVolt = mVolt / 2000.0;
    Ro = Calculate_Rs(mVolt) / Ro_clean_air_factor;
    mVolt = 0.0;
}

void InitGpioInput(gpio_num_t GPIO_PIN){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Kích hoạt pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);
}

void InitGpioOutput(int GPIO_PIN) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);
}

float Get_mVolt(gpio_num_t GPIO_PIN) {
    // Read ADC value
    uint8_t ADC_Value = adc1_get_raw(GPIO_PIN);

    // Calculate voltage in millivolts
    float mVolt = ADC_Value * (Referance_V / 4096.0);
    return mVolt;
}

float Calculate_Rs(float Vo) {
    /* 
    *  Calculate the Rs value
    *  The equation Rs = (Vc - Vo)*(RL/Vo)
    */
    float Rs = (Referance_V - Vo) * (RL / Vo);
    return Rs;
}

void gasMeasurementTask(void){
  for (int i = 0; i < 2000; i++) {
    mVolt += Get_mVolt(MQ6_PIN_ANALOG);
  }
  mVolt = mVolt / 2000.0;
  float Rs = Calculate_Rs(mVolt);
  float Ratio_RsRo = Rs / Ro;
  WARNING_POINT = Ratio_RsRo;
  //printf("%lf\n", Ratio_RsRo);
  ESP_LOGI(TAG_Gas, "Value: %.4f /10", Ratio_RsRo);
  mVolt = 0.0;
}

void InitAM2302Sensor(am2302_handle_t *sensor) {
    am2302_config_t am2302_config = {
        .gpio_num = AM2302_GPIO,
    };

    am2302_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
    };

    ESP_ERROR_CHECK(am2302_new_sensor_rmt(&am2302_config, &rmt_config, sensor));
}

bool Unusual_Point_GAS(){
    if(WARNING_POINT > 11.0 || WARNING_POINT < 7.5) return true;
    return false;
}
