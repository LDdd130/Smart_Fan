#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void Joystick_Init(void);
void Joystick_UpdateServo(void);

#endif