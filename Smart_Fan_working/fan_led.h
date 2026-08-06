#ifndef FNDLED_H
#define FNDLED_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void FanLed_Init(void);
void FanLed_DisplayDuty(uint8_t duty_percent);

#endif