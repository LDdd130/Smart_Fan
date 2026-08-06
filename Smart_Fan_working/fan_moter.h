#ifndef FAN_MOTER_H
#define FAN_MOTER_H

#include <avr/io.h>
#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void FAN_init(void);
void FAN_speed(uint16_t duty);
uint8_t FAN_getDutyPercent(uint16_t duty_cycle);
uint16_t FAN_GetTargetDuty(uint8_t temperature, uint8_t humidity);
uint16_t FAN_RampDuty(uint16_t current_duty, uint16_t target_duty);

#endif