#include "system_timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

static volatile uint32_t system_millis = 0;

void system_timer_init(void)
{
    TCCR2 = (1 << WGM21) | (1 << CS22); // Timer2를 CTC 모드로 설정하고 64분주 클럭을 선택합니다.
    OCR2 = 249; // 16MHz / 64 / (249 + 1) = 1000Hz이므로 비교 일치가 1ms마다 발생하도록 설정합니다.
    TIMSK |= (1 << OCIE2); // Timer2 Output Compare Match 인터럽트를 허용합니다.
}

void system_timer_tick(uint16_t elapsed_ms)
{
    system_millis += elapsed_ms;
}

uint32_t system_millis_get(void)
{
    uint32_t millis;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        millis = system_millis;
    }

    return millis;
}

uint8_t system_millis_elapsed(uint32_t *last_ms, uint16_t interval_ms)
{
    uint32_t now = system_millis_get();

    if ((uint32_t)(now - *last_ms) >= interval_ms) {
        *last_ms = now;
        return 1;
    }

    return 0;
}