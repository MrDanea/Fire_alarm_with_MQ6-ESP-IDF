#pragma once

#include "am2302_rmt.h"

static const char *TAG_Gas = "Nguong khi ga: ";
static const char *TAG_Temp = "Nhiet do, do am: ";


void gasMeasurementTask(void);
void sensor_task(void);
extern float temperature;
extern float humidity;
extern volatile bool WARNING_DELAY;
bool Unusual_Point_GAS();
extern am2302_handle_t sensor;


