#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "main.h"
#include "Task.h"
#include "am2302_rmt.h"
#include "LED.h"

/* Most development boards have "boot" button attached to GPIO0.
 * You can also change this to another pin.
 */
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 \
    || CONFIG_IDF_TARGET_ESP32C6
#define BOOT_BUTTON_NUM         9
#else
#define BOOT_BUTTON_NUM         0
#endif

/* Use boot button as gpio input */
#define GPIO_WAKEUP_NUM_1        BOOT_BUTTON_NUM
/* "Boot" button is active low */
#define GPIO_WAKEUP_LEVEL       0
// #define GPIO_WAKEUP_NUM_CHECK 4
static const char *TAG = "gpio_wakeup";

// void example_wait_gpio_inactive(void)
// {
//     printf("Waiting for GPIO%d to go high...\n", GPIO_WAKEUP_NUM);
//     while (gpio_get_level(GPIO_WAKEUP_NUM) == GPIO_WAKEUP_LEVEL) {
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// } 

void example_wait_gpio_inactive()
{
    printf("Waiting for GPIO%d to go high...\n", GPIO_WAKEUP_NUM_1);
        while (gpio_get_level(GPIO_WAKEUP_NUM_1) == GPIO_WAKEUP_LEVEL) {
            vTaskDelay(pdMS_TO_TICKS(10));

            while(WARNING_DELAY){
                ESP_ERROR_CHECK(am2302_read_temp_humi(sensor, &temperature, &humidity));
                if(!Unusual_Point_GAS()){
                    /*Warning lv 2, LED*/
                    if(temperature >= 40.0 || humidity <= 20.0){
                        /*Tồn tại khí gas, nhiệt độ cao, độ ẩm thấp*/
                        /*đưa cảnh báo lên cấp 3, tiếp tục delay đọc thông số*/
                        
                        warning_task = true;
                        button_stop = false;
                        time_stop = false;
                        /*xuwr lys led*/
                        ESP_LOGI(TAG_Temp, "Temperature: %.1f °C, Humidity: %.1f %%", temperature, humidity);
                        CONTROL_LED_BY_TEMP(temperature);
                    }
                    else {
                        warning_task = true;
                        warning_task = true;
                        button_stop = false;
                        time_stop = false;
                        ESP_LOGI(TAG_Temp, "Temperature: %.1f °C, Humidity: %.1f %%", temperature, humidity);
                        /*Duy trì cảnh báo mức 2, tiếp tục delay đọc thông số*/
                        CONTROL_LED_BY_TEMP(temperature);
                    }
                }
                else {
                    /*Canrh bao gia */
                    CONTROL_LED_BY_TEMP(temperature);
                    printf("FAKE\n");
                    WARNING_DELAY = false;
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
            }

            for(int i = 0; i < 10; i++){
                gasMeasurementTask();
                ESP_ERROR_CHECK(am2302_read_temp_humi(sensor, &temperature, &humidity));
                ESP_LOGI(TAG_Temp, "Temperature: %.1f °C, Humidity: %.1f %%", temperature, humidity);
                CONTROL_LED_BY_TEMP(temperature);
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }

    // if(GPIO_WAKEUP_NUM_1 == 0x100){
    //     printf("GPIO8");
        
    // }
    // else if(GPIO_WAKEUP_NUM_1 == 0x200) printf("GPIO9");
    // else printf("fail");
    
}

esp_err_t example_register_gpio_wakeup()
{
    /* Initialize GPIO */
    gpio_config_t config = {
            .pin_bit_mask = BIT64(GPIO_WAKEUP_NUM_1),
            .mode = GPIO_MODE_INPUT,
            .pull_down_en = false,
            .pull_up_en = false,
            .intr_type = GPIO_INTR_DISABLE
    };
    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "Initialize GPIO%d failed", GPIO_WAKEUP_NUM_1);

    /* Enable wake up from GPIO */
    ESP_RETURN_ON_ERROR(gpio_wakeup_enable(GPIO_WAKEUP_NUM_1, GPIO_WAKEUP_LEVEL == 0 ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL),
                        TAG, "Enable gpio wakeup failed");
    ESP_RETURN_ON_ERROR(esp_sleep_enable_gpio_wakeup(), TAG, "Configure gpio as wakeup source failed");

    /* Make sure the GPIO is inactive and it won't trigger wakeup immediately */
    example_wait_gpio_inactive();
    
    ESP_LOGI(TAG, "gpio wakeup source is ready");

    return ESP_OK;
}
