#include "fan_led.h"

static const uint8_t fan_led_bar[FAN_LED_COUNT] = {
    0x80,
    0xC0,
    0xE0,
    0xF0,
    0xF8,
    0xFC,
    0xFE,
    0xFF
};

// PORTA에 연결된 LED bar를 출력으로 초기화한다.
// FAN_LED_PORT를 0으로 초기화해 부팅 직후에는 모든 LED가 꺼진 상태에서 시작한다.
void FanLed_Init(void)
{
    // main_adc.c의 array LED 방식과 동일하게 PORTA 전체를 LED bar 출력으로 사용한다.
    FAN_LED_DDR = 0xFF;
    FAN_LED_PORT = 0x00;
}

// 현재 팬 Duty(%)를 LED bar 점등 개수로 변환해 표시한다.
// fan_led_bar[]는 누적 점등 패턴이므로 led_count가 3이면 0xE0 패턴을 출력한다.
void FanLed_DisplayDuty(uint8_t duty_percent)
{
    uint8_t led_count = 0;

    // 팬이 실제로 돌지 않는 0~24% 구간은 LED도 꺼 둔다.
    // 25~100% 구간을 LED 1~8칸으로 매핑해 팬 파워를 표시한다.
    if (duty_percent >= FAN_START_DUTY_PERCENT) {
        led_count = 1 + ((uint16_t)(duty_percent - FAN_START_DUTY_PERCENT) * (FAN_LED_COUNT - 1))
                      / (100 - FAN_START_DUTY_PERCENT);
    }

    FAN_LED_PORT = (led_count == 0) ? 0x00 : fan_led_bar[led_count - 1];
}