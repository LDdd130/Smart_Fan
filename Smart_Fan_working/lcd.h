#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "app_config.h"
#include "pinmap.h"

// --- [1. I2C(TWI) 제어 함수] ---

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(unsigned char data);

// --- [2. I2C LCD 1602 제어 함수] ---

void LCD_Write_4bit(unsigned char data, unsigned char rs);
void LCD_Init(void);
void LCD_WriteString(const char *str);
void FAN_init(void);
void FAN_speed(uint16_t duty);
uint8_t FAN_getDutyPercent(uint16_t duty_cycle);
uint16_t FAN_GetTargetDuty(uint8_t temperature, uint8_t humidity);
uint16_t FAN_RampDuty(uint16_t current_duty, uint16_t target_duty);
void LCD_PrintStatus(uint8_t temperature, uint8_t humidity, uint16_t fan_duty, uint16_t target_duty);
void LCD_PrintDhtError(void);

#endif