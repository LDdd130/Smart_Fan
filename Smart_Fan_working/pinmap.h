#ifndef PINMAP_H
#define PINMAP_H

#include <avr/io.h>

// Button
#define BUTTON_DDR   DDRG
#define BUTTON_PIN   PING
#define BUTTON_ON    2
#define BUTTON_OFF   3
#define BUTTON_TOGGLE 4

// LED
#define LED_DDR      DDRD
#define LED_PORT     PORTD

// 4FND segment and digit select
#define FND_SEG_DDR     DDRC
#define FND_SEG_PORT    PORTC
#define FND_DIGIT_DDR   DDRB
#define FND_DIGIT_PORT  PORTB
#define FND_DIGIT_MASK  0x1E
#define FND_DIGIT0_PIN  PB1
#define FND_DIGIT1_PIN  PB2
#define FND_DIGIT2_PIN  PB3
#define FND_DIGIT3_PIN  PB4

// Timer/fan shared motor output
#define MOTOR_DDR   DDRB
#define MOTOR_PORT  PORTB
#define MOTOR_PIN   PB5

// Servo PWM outputs
#define SERVO_DDR   DDRB
#define SERVO_PIN   PB6
#define SERVO2_DDR  DDRE
#define SERVO2_PIN  PE3

// Joystick ADC inputs
#define JOYSTICK_DDR   DDRF
#define JOYSTICK_PORT  PORTF
#define JOYSTICK_X_CH  0
#define JOYSTICK_Y_CH  1

// DHT sensor data line
#define DHT_DDR   DDRB
#define DHT_PORT  PORTB
#define DHT_PIN   PINB
#define DHT_DATA  PB0

// Fan power indicator LED bar
#define FAN_LED_DDR   DDRA
#define FAN_LED_PORT  PORTA

#endif