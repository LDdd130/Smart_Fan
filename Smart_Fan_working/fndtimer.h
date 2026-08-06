#ifndef FNDTIMER_H
#define FNDTIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "app_config.h"
#include "pinmap.h"

void uart0_init(void); // UART0를 초기화해 PC/터미널에서 타이머 문자열을 받을 수 있게 합니다.
void fnd_init(void); // 4FND 출력 핀과 Timer2 인터럽트를 초기화합니다.
uint8_t uart0_transmit_ready(void); // UART0 송신 버퍼에 새 데이터를 쓸 수 있는지 확인합니다.
void uart0_transmit(char data); // UART0로 문자 1바이트를 송신합니다.
uint8_t uart0_receive_ready(void); // UART0로 수신된 문자가 있는지 확인합니다.
uint8_t uart0_receive(void); // UART0 수신 버퍼에서 문자 1바이트를 읽습니다.
void fnd_display_digit(uint8_t digit, uint8_t number); // 특정 FND 자리 하나에 숫자 하나를 직접 표시합니다.
void motor_set_fan_request(uint8_t enabled); // 팬 제어 로직이 모터 출력을 요구하는지 저장합니다.
void motor_update(uint16_t timer_seconds); // 타이머 남은 시간과 팬 요청을 기준으로 모터 핀을 갱신합니다.
uint8_t timer_parse_input(char *buffer, uint8_t length, uint16_t *timer_seconds); // UART 입력 문자열을 전체 초 단위 타이머 값으로 변환합니다.

uint8_t timer_uart_update(uint16_t *timer_seconds); // UART 입력을 읽고 정상 입력이면 타이머 값을 갱신합니다.

void timer_display_update(uint16_t timer_seconds); // 전체 초 값을 분/초 네 자리 숫자로 나누어 4FND 표시 버퍼에 저장합니다.

#endif