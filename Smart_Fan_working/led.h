#include <stdio.h>      // printf(), scanf() 함수 사용하기 위한 헤더파일
#include <stdint.h>     // uint8_t, uint16_t, uint32_t, ...
#include <stdbool.h>    // bool, true, false
#include <stdarg.h>     // 가변인자 함수 사용하기 위한 헤더파일
#include <stdlib.h>     // malloc, free, atoi, itoa, ...
#include <avr/io.h>     // AVR 입출력 레지스터 정의
#include <util/delay.h> // _delay_ms(), _delay_us() 함수 사용하기 위한 헤더파일

#include "pinmap.h"

typedef struct
{
    volatile uint8_t *port;     // LED가 연결된 포트
    uint8_t            pin;     // LED가 연결된 핀번호
}LED;

void ledInit(LED *led);
void ledOn(LED *led);
void ledOff(LED *led);