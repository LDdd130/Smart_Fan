#include "joystick.h"

#include <avr/io.h>

#include "servo.h"

void Joystick_Init(void)
{
    JOYSTICK_DDR &= ~(1 << JOYSTICK_X_CH);
    JOYSTICK_DDR &= ~(1 << JOYSTICK_Y_CH);
    JOYSTICK_PORT &= ~(1 << JOYSTICK_X_CH);
    JOYSTICK_PORT &= ~(1 << JOYSTICK_Y_CH);

    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

static uint16_t ADC_Read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}

void Joystick_UpdateServo(void)
{
    uint16_t joystickX = ADC_Read(JOYSTICK_X_CH);
    uint16_t joystickY = ADC_Read(JOYSTICK_Y_CH);
    uint16_t step;

    if (joystickX < (JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE))
    {
        step = 1 + ((JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE - joystickX) / 120);
        if (step > SERVO_MAX_STEP)
        {
            step = SERVO_MAX_STEP;
        }
        Servo_MoveHorizontalUp(step);
    }
    else if (joystickX > (JOYSTICK_CENTER + JOYSTICK_DEAD_ZONE))
    {
        step = 1 + ((joystickX - JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE) / 120);
        if (step > SERVO_MAX_STEP)
        {
            step = SERVO_MAX_STEP;
        }
        Servo_MoveHorizontalDown(step);
    }

    if (joystickY < (JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE))
    {
        step = 1 + ((JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE - joystickY) / 120);
        if (step > SERVO_MAX_STEP)
        {
            step = SERVO_MAX_STEP;
        }
        Servo_MoveVerticalUp(step);
    }
    else if (joystickY > (JOYSTICK_CENTER + JOYSTICK_DEAD_ZONE))
    {
        step = 1 + ((joystickY - JOYSTICK_CENTER - JOYSTICK_DEAD_ZONE) / 120);
        if (step > SERVO_MAX_STEP)
        {
            step = SERVO_MAX_STEP;
        }
        Servo_MoveVerticalDown(step);
    }

    Servo_Apply();
}