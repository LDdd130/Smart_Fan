#include "fan_moter.h"
#include "fndtimer.h"
#include <avr/io.h>

static uint8_t motor_fan_request = 0; 
static uint16_t current_sensor_duty = 0; // 센서가 요구한 현재 속도 비율을 저장합니다.

void FAN_init(void)
{
    MOTOR_DDR |= (1 << MOTOR_PIN);
    
    // Timer1의 채널A(PB5) PWM 출력을 활성화합니다.
    TCCR1A = (TCCR1A & ~((1 << COM1A1) | (1 << COM1A0))) | (1 << COM1A1); 

    // 만약 시스템 내 다른 곳(서보모터 등)에서 Timer1을 켜지 않았다면 기본 모드로 가동
    if ((TCCR1B & 0x07) == 0) {
        TCCR1A |= (1 << WGM10); 
        TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10); 
    }
    
    motor_set_fan_request(0);
    current_sensor_duty = 0;
}

void FAN_speed(uint16_t duty_cycle)
{
    if (duty_cycle > FAN_PWM_TOP) {
        duty_cycle = FAN_PWM_TOP;
    }

    motor_set_fan_request(duty_cycle > 0);
    current_sensor_duty = duty_cycle; // 값만 저장해두고, 실제 핀 반영은 motor_update가 담당합니다.
}

uint8_t FAN_getDutyPercent(uint16_t duty_cycle)
{
    return (uint32_t)duty_cycle * 100 / FAN_PWM_TOP;
}

uint16_t FAN_MapRangeToDuty(uint8_t value, uint8_t min_value, uint8_t max_value)
{
    if (value <= min_value) {
        return 0; 
    }
    if (value >= max_value) {
        return FAN_PWM_TOP; 
    }

    // [해결 포인트 1] 50Hz PWM 파형 특성 반영
    // 서보모터와 공유할 경우 50Hz의 느린 주파수로 펄스가 들어옵니다. 
    // 관성을 유지해 덜덜거림을 막으려면 1단계일 때 최소 45% 이상의 힘으로 쳐주어야 합니다.
    uint16_t min_pwm = (FAN_PWM_TOP * 45) / 100;
    
    return min_pwm + (uint32_t)(value - min_value) * (FAN_PWM_TOP - min_pwm) / (max_value - min_value);
}

uint16_t FAN_GetTargetDuty(uint8_t temperature, uint8_t humidity)
{
    uint16_t temp_duty = FAN_MapRangeToDuty(temperature, TEMP_THRESHOLD, TEMP_MAX);
    uint16_t hum_duty = FAN_MapRangeToDuty(humidity, HUM_THRESHOLD, HUM_MAX);

    return (temp_duty > hum_duty) ? temp_duty : hum_duty;
}

uint16_t FAN_RampDuty(uint16_t current_duty, uint16_t target_duty)
{
    if (current_duty < target_duty) {
        uint16_t next_duty = current_duty + FAN_RAMP_STEP;
        return (next_duty > target_duty) ? target_duty : next_duty;
    }

    if (current_duty > target_duty) {
        if (current_duty < FAN_RAMP_STEP) {
            return target_duty;
        }
        uint16_t next_duty = current_duty - FAN_RAMP_STEP;
        return (next_duty < target_duty) ? target_duty : next_duty;
    }

    return current_duty;
}

void motor_set_fan_request(uint8_t is_requested)
{
    motor_fan_request = is_requested;
}

uint8_t motor_get_fan_request(void)
{
    return motor_fan_request;
}

void motor_update(uint16_t timer_seconds)
{
    // [해결 포인트 2] 마법의 스케일링 로직!
    // 서보모터(Timer1)가 가동 중인지 체크하여, 실제 최대값(TOP)을 알아냅니다.
    uint32_t top_val = 255;
    if (TCCR1B & (1 << WGM13)) {
        top_val = ICR1; // 서보모터가 바꾼 수만 단위의 거대한 TOP 값 적용
        if (top_val == 0) top_val = 255;
    }

    if (timer_seconds > 0) {
        // UART 수동 타이머로 켤 때는 100% 강제 가동
        OCR1A = top_val;
    } else if (motor_fan_request) {
        // 온습도 센서에 의해 켜질 때는 칸수(비율)에 맞게 수만 단위의 값을 자동으로 곱셈 처리!
        // 예: 8칸(100%) = ICR1 전체 인가, 4칸(50%) = ICR1 절반 인가
        OCR1A = (uint16_t)((top_val * current_sensor_duty) / FAN_PWM_TOP);
    } else {
        // 요청이 없으면 확실하게 0을 넣어 정지
        OCR1A = 0; 
    }
}