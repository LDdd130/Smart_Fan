#include "servo.h"

#include <avr/io.h>

static uint16_t servoHorizontalDuty = SERVO_90_DEG;
static uint16_t servoVerticalDuty = SERVO_90_DEG;

static void PWM_Init(void)
{
    TCCR1A = (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
    TCCR1C = 0;
    ICR1 = 4999;
}

static void PWM3_Init(void)
{
    TCCR3A = (1 << COM3A1) | (1 << WGM31);
    TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS31) | (1 << CS30);
    TCCR3C = 0;
    ICR3 = 4999;
}

void Servo_Apply(void)
{
    OCR1B = servoHorizontalDuty;
    OCR3A = servoVerticalDuty;
}

void Servo_Init(void)
{
    SERVO_DDR |= (1 << SERVO_PIN);
    SERVO2_DDR |= (1 << SERVO2_PIN);

    PWM_Init();
    PWM3_Init();
    Servo_Apply();
}

void Servo_MoveHorizontalUp(uint16_t step)
{
    if (servoHorizontalDuty + step < SERVO_180_DEG)
    {
        servoHorizontalDuty += step;
    }
    else
    {
        servoHorizontalDuty = SERVO_180_DEG;
    }
}

void Servo_MoveHorizontalDown(uint16_t step)
{
    if (servoHorizontalDuty > SERVO_0_DEG + step)
    {
        servoHorizontalDuty -= step;
    }
    else
    {
        servoHorizontalDuty = SERVO_0_DEG;
    }
}

void Servo_MoveVerticalUp(uint16_t step)
{
    if (servoVerticalDuty + step < SERVO_180_DEG)
    {
        servoVerticalDuty += step;
    }
    else
    {
        servoVerticalDuty = SERVO_180_DEG;
    }
}

void Servo_MoveVerticalDown(uint16_t step)
{
    if (servoVerticalDuty > SERVO_0_DEG + step)
    {
        servoVerticalDuty -= step;
    }
    else
    {
        servoVerticalDuty = SERVO_0_DEG;
    }
}

void Servo_UpdateAutoHorizontal(uint8_t isEnabled)
{
    static uint8_t direction = 1;

    if (!isEnabled)
    {
        Servo_Apply();
        return;
    }

    if (direction)
    {
        Servo_MoveHorizontalUp(SERVO_AUTO_STEP);
        if (servoHorizontalDuty == SERVO_180_DEG)
        {
            direction = 0;
        }
    }
    else
    {
        Servo_MoveHorizontalDown(SERVO_AUTO_STEP);
        if (servoHorizontalDuty == SERVO_0_DEG)
        {
            direction = 1;
        }
    }

    Servo_Apply();
}

void Servo_UpdateAutoVertical(uint8_t isEnabled)
{
    static uint8_t direction = 1;

    if (!isEnabled)
    {
        Servo_Apply();
        return;
    }

    if (direction)
    {
        Servo_MoveVerticalUp(SERVO_AUTO_STEP);
        if (servoVerticalDuty == SERVO_180_DEG)
        {
            direction = 0;
        }
    }
    else
    {
        Servo_MoveVerticalDown(SERVO_AUTO_STEP);
        if (servoVerticalDuty == SERVO_0_DEG)
        {
            direction = 1;
        }
    }

    Servo_Apply();
}