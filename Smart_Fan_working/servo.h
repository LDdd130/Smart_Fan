#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void Servo_Init(void);
void Servo_UpdateAutoHorizontal(uint8_t isEnabled);
void Servo_UpdateAutoVertical(uint8_t isEnabled);
void Servo_MoveHorizontalUp(uint16_t step);
void Servo_MoveHorizontalDown(uint16_t step);
void Servo_MoveVerticalUp(uint16_t step);
void Servo_MoveVerticalDown(uint16_t step);
void Servo_Apply(void);

#endif