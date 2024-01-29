#pragma once

#ifndef LED_H
#define LED_H

#define TEM_5dC 0x01 
#define TEM_10dC 0x03
#define TEM_15dC 0x07
#define TEM_20dC 0x0f
#define TEM_25dC 0x1F
#define TEM_30dC 0x3F
#define TEM_35dC 0x7F
#define TEM_40dC 0xFF
#define HIGH 0x1
#define LOW 0x0
#define stcp_Pin 7
#define shcp_Pin 6
#define ds_Pin 3
#define MSBFIRST 0
#define LSBFIRST 1

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value);
void CONTROL_LED_BY_TEMP(float temp);
void TURN_LIGHT(int temp);

#endif // LED_H