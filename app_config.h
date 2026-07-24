#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// UART timer input
#define UART_INPUT_MAX 5
#define UART_INPUT_TIMEOUT 500

// System timer tick used by the current Timer2 ISR.
#define SYSTEM_TIMER_TICK_MS 4

// Button
#define BUTTON_DEBOUNCE_COUNT 30

// Servo
#define SERVO_0_DEG      125
#define SERVO_90_DEG     375
#define SERVO_180_DEG    625
#define SERVO_AUTO_STEP  1
#define SERVO_UPDATE_INTERVAL_MS 20

// Joystick
#define JOYSTICK_CENTER     512
#define JOYSTICK_DEAD_ZONE  40
#define SERVO_MAX_STEP      5

// Fan LED
#define FAN_LED_COUNT 8

// Fan control mapping
#define FAN_PWM_TOP 639
#define FAN_START_DUTY_PERCENT 25
#define TEMP_THRESHOLD 28
#define HUM_THRESHOLD 50
#define TEMP_MAX 30
#define HUM_MAX 90
#define FAN_RAMP_STEP 16

// Periodic task intervals
#define SENSOR_READ_INTERVAL_MS 2000
#define FAN_RAMP_INTERVAL_MS 100
#define TIMER_COUNTDOWN_INTERVAL_MS 1000

#endif
