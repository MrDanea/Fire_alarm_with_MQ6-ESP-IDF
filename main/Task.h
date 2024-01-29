#pragma once

#include "freertos/timers.h"

extern volatile bool warning_task;
extern volatile bool time_stop;
extern volatile bool button_stop;

void buzzer_task(void *pvParameters);
void button_task(void *pvParameter);
void warningbtn_task(void *pvParameter);
bool STOP(bool time, bool button, bool warning);
void debounce_timer_callback(TimerHandle_t xTimer);
void debounce_timer_callback_2(TimerHandle_t xTimer);