#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LED.h"

void TURN_LIGHT(int temp){
    gpio_set_level(stcp_Pin, LOW);
    shiftOut(ds_Pin, shcp_Pin, MSBFIRST, temp);
    gpio_set_level(stcp_Pin, HIGH);
}

void CONTROL_LED_BY_TEMP(float temp){
    if(temp <= 5.0) TURN_LIGHT(TEM_5dC);
    else if(temp > 5.0 && temp <= 10.0) TURN_LIGHT(TEM_10dC);
    else if(temp > 10.0 && temp <= 15.0) TURN_LIGHT(TEM_15dC);
    else if(temp > 15.0 && temp <= 20.0) TURN_LIGHT(TEM_20dC);
    else if(temp > 20.0 && temp <= 25.0) TURN_LIGHT(TEM_25dC);
    else if(temp > 25.0 && temp <= 30.0) TURN_LIGHT(TEM_30dC);
    else if(temp > 30.0 && temp <= 35.0) TURN_LIGHT(TEM_35dC);
    else if(temp > 35.0 && temp <= 40.0) TURN_LIGHT(TEM_40dC);
    else TURN_LIGHT(TEM_40dC);
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST) {
            gpio_set_level(dataPin, !!(value & (1 << i)));
        } else {
            gpio_set_level(dataPin, !!(value & (1 << (7 - i))));
        }

        gpio_set_level(clockPin, HIGH);
        gpio_set_level(clockPin, LOW);
    }
}
