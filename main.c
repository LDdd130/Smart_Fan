#define F_CPU 16000000UL

#include <stdint.h>
#include <util/delay.h>
#include "button.h"
#include "joystick.h"
#include "servo.h"
#include "lcd.h"
#include "fndtimer.h"
#include "dht.h"
#include "fan_led.h"
#include "system_timer.h"

int main(void)
{
    BUTTON btnHorizontal;
    BUTTON btnVertical;
    uint8_t horizontalAuto = 0;
    uint8_t verticalAuto = 0;
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    uint16_t fan_duty = 0;
    uint16_t target_duty = 0;
    uint16_t timer_seconds = 0; // 4FND에 MMSS 형식으로 표시하고 카운트다운할 남은 시간을 초 단위로 저장합니다.
    uint32_t sensor_last_ms = 0; // 센서 읽기 주기를 system_millis 기준으로 계산하기 위한 마지막 실행 시각입니다.
    uint32_t ramp_last_ms = 0; // 팬 듀티를 서서히 바꾸는 주기를 system_millis 기준으로 계산하기 위한 마지막 실행 시각입니다.
    uint32_t second_last_ms = 0; // 타이머를 1초마다 감소시키기 위한 마지막 실행 시각입니다.
    uint32_t servo_last_ms = 0; // 서보 갱신 주기를 system_millis 기준으로 계산하기 위한 마지막 실행 시각입니다.

    ButtonInit(&btnHorizontal, &BUTTON_DDR, &BUTTON_PIN, BUTTON_ON);
    ButtonInit(&btnVertical, &BUTTON_DDR, &BUTTON_PIN, BUTTON_OFF);
    Servo_Init();
    Joystick_Init();

    fnd_init(); // 4FND 세그먼트/자리 선택 핀과 Timer2 인터럽트를 초기화합니다.
    uart0_init(); // UART로 타이머 값을 입력받을 수 있도록 USART0를 초기화합니다.
    motor_update(timer_seconds); // 초기 타이머 값이 0이므로 타이머 연동 모터 출력을 꺼진 상태로 맞춥니다.
    timer_display_update(timer_seconds); // 초기 0초 값을 4FND 표시 버퍼에 00:00 형태로 반영합니다.
    sei(); // Timer2 ISR이 4FND 동적 표시와 system_millis 증가를 시작할 수 있도록 전역 인터럽트를 켭니다.

    I2C_Init();
    LCD_Init();
    FAN_init();
    FanLed_Init();
    DHT_SetInput();
    FAN_speed(0);
    FanLed_DisplayDuty(0);
    LCD_Write_4bit(0x80, 0);
    LCD_WriteString("DHT Fan Control");
    LCD_Write_4bit(0xC0, 0);
    LCD_WriteString("Starting...");
    _delay_ms(1000);

    while (1)
    {
        if(timer_uart_update(&timer_seconds)) // UART 입력으로 새 타이머 값이 들어왔는지 확인합니다.
        {
            second_last_ms = system_millis_get(); // 새 시간 입력 직후 바로 1초가 깎이지 않도록 기준 시각을 현재로 맞춥니다.
            motor_update(timer_seconds); // 새 타이머 값이 0보다 크면 타이머 연동 모터 출력을 켭니다.
            timer_display_update(timer_seconds); // 새 타이머 값을 4FND 표시 버퍼에 즉시 반영합니다.
        }

        if (system_millis_elapsed(&sensor_last_ms, SENSOR_READ_INTERVAL_MS)) // Timer2 기반 시간으로 센서 읽기 주기가 지났는지 확인합니다.
        {
            if (DHT_Read(&temperature, &humidity))
            {
                target_duty = FAN_GetTargetDuty(temperature, humidity);
                LCD_PrintStatus(temperature, humidity, fan_duty, target_duty);
            }
            else
            {
                LCD_PrintDhtError();
            }
        }

        if (system_millis_elapsed(&ramp_last_ms, FAN_RAMP_INTERVAL_MS)) // Timer2 기반 시간으로 팬 듀티 갱신 주기가 지났는지 확인합니다.
        {
            fan_duty = FAN_RampDuty(fan_duty, target_duty);
            FAN_speed(fan_duty);
            motor_update(timer_seconds); // 팬 동작 요청과 타이머 상태를 합쳐 모터 출력 상태를 갱신합니다.
            FanLed_DisplayDuty(FAN_getDutyPercent(fan_duty));
        }

        if (ButtonGetState(&btnHorizontal) == ACT_PUSH)
        {
            horizontalAuto = !horizontalAuto;
        }
        if (ButtonGetState(&btnVertical) == ACT_PUSH)
        {
            verticalAuto = !verticalAuto;
        }

        if(system_millis_elapsed(&servo_last_ms, SERVO_UPDATE_INTERVAL_MS)) // Timer2 기반 시간으로 서보 제어를 약 20ms 간격으로 실행합니다.
        {
            if (!horizontalAuto && !verticalAuto)
            {
                Joystick_UpdateServo();
            }
            else
            {
                Servo_UpdateAutoHorizontal(horizontalAuto);
                Servo_UpdateAutoVertical(verticalAuto);
            }
        }

        if(system_millis_elapsed(&second_last_ms, TIMER_COUNTDOWN_INTERVAL_MS)) // Timer2 기반 시간으로 1초가 지났는지 확인해 카운트다운을 진행합니다.
        {
            if(timer_seconds > 0) timer_seconds--; // 남은 시간이 있으면 1초를 감소시킵니다.
            motor_update(timer_seconds); // 감소 후 남은 시간이 0이면 타이머 연동 모터 출력을 끕니다.
            timer_display_update(timer_seconds); // 감소한 값을 4FND 표시 버퍼에 반영합니다.
        }
    }
}