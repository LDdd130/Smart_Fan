#include <util/atomic.h>

#include "fndtimer.h"
#include "system_timer.h"

static const uint8_t fndNumber[] =
{   0x3F, // 숫자 0을 7세그먼트에 표시하기 위한 세그먼트 비트 패턴입니다.
    0x06, // 숫자 1을 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x5B, // 숫자 2를 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x4F, // 숫자 3을 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x66, // 숫자 4를 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x6D, // 숫자 5를 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x7D, // 숫자 6을 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x07, // 숫자 7을 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x7F, // 숫자 8을 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
    0x6F  // 숫자 9를 표시할 때 켜야 하는 세그먼트 비트 패턴입니다.
};

static const uint8_t fnd_sel[] =
{   (uint8_t)~(1 << FND_DIGIT0_PIN), // 첫 번째 FND 자리 선택선만 0으로 만들어 해당 자리를 켜는 마스크입니다.
    (uint8_t)~(1 << FND_DIGIT1_PIN), // 두 번째 FND 자리 선택선만 0으로 만들어 해당 자리를 켜는 마스크입니다.
    (uint8_t)~(1 << FND_DIGIT2_PIN), // 세 번째 FND 자리 선택선만 0으로 만들어 해당 자리를 켜는 마스크입니다.
    (uint8_t)~(1 << FND_DIGIT3_PIN)  // 네 번째 FND 자리 선택선만 0으로 만들어 해당 자리를 켜는 마스크입니다.
};

static volatile uint8_t fnd_display_digits[4] = { 0, 0, 0, 0 }; // ISR이 실제로 표시할 4자리 숫자 값을 저장하는 버퍼입니다.
static volatile uint8_t fnd_scan_digit = 0; // 현재 ISR에서 갱신할 FND 자리 번호(0~3)를 저장합니다.

void fnd_init(void)
{
    FND_SEG_DDR = 0xFF; // PORTC 전체를 7세그먼트 세그먼트 출력 핀으로 사용하기 위해 출력으로 설정합니다.
    FND_DIGIT_DDR |= FND_DIGIT_MASK; // FND 자리 선택 핀을 출력으로 설정합니다.
    MOTOR_DDR |= (1 << MOTOR_PIN); // 모터 제어 핀을 출력으로 설정합니다.
    FND_DIGIT_PORT |= FND_DIGIT_MASK; // 모든 FND 자리를 꺼진 상태로 초기화합니다.
    MOTOR_PORT &= ~(1 << MOTOR_PIN); // 모터 출력은 꺼진 상태로 초기화합니다.
    system_timer_init(); // FND 동적 표시와 ms 시간 측정에 사용할 Timer2 인터럽트를 설정합니다.
}

ISR(TIMER2_COMP_vect)
{
    uint8_t digit_port_keep = FND_DIGIT_PORT & ~FND_DIGIT_MASK; // FND 자리 선택 비트만 지우고 나머지 포트 상태는 보존합니다.

    system_timer_tick(SYSTEM_TIMER_TICK_MS); // 공용 시스템 시간 값을 누적합니다.

    FND_DIGIT_PORT = digit_port_keep | FND_DIGIT_MASK; // 잔상 방지를 위해 숫자 패턴을 바꾸기 전에 네 자리 FND를 모두 끕니다.
    FND_SEG_PORT = fndNumber[fnd_display_digits[fnd_scan_digit]]; // 현재 스캔할 자리의 숫자를 세그먼트 비트 패턴으로 변환해 출력합니다.
    FND_DIGIT_PORT = digit_port_keep | (fnd_sel[fnd_scan_digit] & FND_DIGIT_MASK); // 현재 자리 선택선만 활성화해서 해당 FND 자리만 켭니다.

    fnd_scan_digit++; // 다음 인터럽트에서 갱신할 자리로 이동합니다.
    if(fnd_scan_digit >= 4) {
        fnd_scan_digit = 0; // 네 번째 자리까지 표시했으면 다시 첫 번째 자리부터 반복합니다.
    }
}

void uart0_init(void)
{
    UCSR0A |= (1 << U2X0); // USART0를 2배속 모드로 설정해 보드레이트 오차를 줄입니다.
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // USART0 수신기와 송신기를 모두 활성화합니다.
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); // 데이터 프레임을 8비트 데이터 형식으로 설정합니다.
    UBRR0H = 0; // 보드레이트 레지스터의 상위 바이트를 0으로 설정합니다.
    UBRR0L = 207; // 16MHz, 2배속 기준 9600bps에 맞는 보드레이트 값을 설정합니다.
}

uint8_t uart0_transmit_ready(void)
{
    return (UCSR0A & (1 << UDRE0)); // 송신 데이터 레지스터가 비었으면 0이 아닌 값을 반환합니다.
}

void uart0_transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0))); // 이전 송신이 끝나 UDR0에 새 데이터를 쓸 수 있을 때까지 기다립니다.
    UDR0 = data; // 전송할 1바이트 문자를 USART0 데이터 레지스터에 씁니다.
}

uint8_t uart0_receive_ready(void)
{
    return (UCSR0A & (1 << RXC0)); // 수신 완료 플래그가 서 있으면 읽을 데이터가 있다는 뜻입니다.
}

uint8_t uart0_receive(void)
{
    return UDR0; // USART0 수신 버퍼에서 1바이트 데이터를 읽어 반환합니다.
}

void fnd_display_digit(uint8_t digit, uint8_t number)
{
    uint8_t digit_port_keep = FND_DIGIT_PORT & ~FND_DIGIT_MASK; // FND 자리 선택 비트만 제외하고 기존 출력 상태를 보존합니다.

    FND_DIGIT_PORT = digit_port_keep | FND_DIGIT_MASK; // 숫자를 바꾸기 전에 모든 FND 자리를 꺼서 잔상을 줄입니다.
    FND_SEG_PORT = fndNumber[number]; // 표시할 숫자를 세그먼트 패턴으로 바꿔 출력합니다.
    FND_DIGIT_PORT = digit_port_keep | (fnd_sel[digit] & FND_DIGIT_MASK); // 요청한 자리만 선택해서 해당 숫자를 표시합니다.
}

uint8_t timer_parse_input(char *buffer, uint8_t length, uint16_t *timer_seconds)
{
    uint16_t minutes = 0; // 입력 문자열에서 해석한 분 값을 누적합니다.
    uint8_t seconds = 0; // 입력 문자열에서 해석한 초 값을 누적합니다.
    uint8_t colon = 0; // ':' 문자를 이미 만났는지 기록해 분/초 영역을 구분합니다.
    uint8_t minute_digits = 0; // 분 영역에 입력된 숫자 개수를 기록합니다.
    uint8_t second_digits = 0; // 초 영역에 입력된 숫자 개수를 기록합니다.

    for(uint8_t i = 0; i < length; i++) {
        char data = buffer[i]; // 버퍼에서 현재 검사할 문자 하나를 가져옵니다.

        if(data >= '0' && data <= '9') {
            if(colon) {
                seconds = seconds * 10 + (data - '0'); // ':' 뒤의 숫자는 초 값으로 누적합니다.
                second_digits++; // 초 영역에 입력된 숫자 개수를 하나 증가시킵니다.
                if(seconds >= 60 || second_digits > 2) return 0; // 초는 0~59, 최대 두 자리만 허용합니다.
            } else {
                minutes = minutes * 10 + (data - '0'); // ':' 앞의 숫자는 분 값으로 누적합니다.
                minute_digits++; // 분 영역에 입력된 숫자 개수를 하나 증가시킵니다.
                if(minutes > 99 || minute_digits > 2) return 0; // 분은 0~99, 최대 두 자리만 허용합니다.
            }
        } else if(data == ':' && colon == 0) {
            colon = 1; // 첫 번째 ':'를 만나면 이후 숫자를 초 영역으로 해석합니다.
        } else {
            return 0; // 숫자나 첫 ':'가 아닌 문자가 오면 잘못된 입력으로 처리합니다.
        }
    }

    if(minute_digits == 0 && !colon) return 0; // ':'가 없는 입력에서는 최소 한 자리 이상의 분 숫자가 필요합니다.
    if(colon && second_digits == 0) return 0; // ':'가 있는 입력에서는 초 숫자가 반드시 뒤따라야 합니다.

    *timer_seconds = (minutes * 60) + seconds; // 해석한 분과 초를 전체 초 단위로 변환해 저장합니다.
    return 1; // 입력 해석에 성공했음을 알립니다.
}

uint8_t timer_uart_update(uint16_t *timer_seconds)
{
    static char uart_input[UART_INPUT_MAX]; // UART로 받은 타이머 입력 문자열을 임시 저장하는 버퍼입니다.
    static uint8_t uart_input_len = 0; // 현재 버퍼에 저장된 문자 개수입니다.
    static uint8_t uart_input_wait = 0; // 입력이 멈춘 뒤 자동 파싱까지 기다린 반복 횟수입니다.
    uint8_t updated = 0; // 이번 호출에서 timer_seconds가 새 값으로 갱신되었는지 나타내는 플래그입니다.

    while(uart0_receive_ready()) {
        char rx_data = uart0_receive(); // UART 수신 버퍼에서 새 문자 하나를 읽어옵니다.

        uart0_transmit(rx_data); // 사용자가 입력한 문자를 터미널에 다시 보내 에코처럼 보이게 합니다.

        if(rx_data == '\r' || rx_data == '\n') {
            if(uart_input_len > 0) {
                if(timer_parse_input(uart_input, uart_input_len, timer_seconds)) {
                    updated = 1; // 엔터 전까지 모은 입력이 정상 형식이면 타이머 값 갱신을 표시합니다.
                }

                uart_input_len = 0; // 입력 처리가 끝났으므로 버퍼 길이를 초기화합니다.
            }

            uart_input_wait = 0; // 엔터를 받았으므로 자동 파싱 대기 카운터를 초기화합니다.
        } else if((rx_data >= '0' && rx_data <= '9') || rx_data == ':') {
            if(uart_input_len < UART_INPUT_MAX) {
                uart_input[uart_input_len++] = rx_data; // 숫자 또는 ':' 문자는 타이머 입력 버퍼에 저장합니다.
                uart_input_wait = 0; // 새 문자가 들어왔으므로 입력 정지 대기 시간을 다시 셉니다.
            } else {
                uart_input_len = 0; // 버퍼가 꽉 찬 상태에서 더 입력되면 잘못된 입력으로 보고 버퍼를 비웁니다.
                uart_input_wait = 0; // 버퍼를 비웠으므로 대기 카운터도 초기화합니다.
            }
        } else {
            uart_input_len = 0; // 허용하지 않는 문자가 들어오면 현재까지의 입력을 폐기합니다.
            uart_input_wait = 0; // 잘못된 입력 후 자동 파싱이 일어나지 않도록 대기 카운터를 초기화합니다.
        }
    }

    if(uart_input_len > 0) {
        uart_input_wait++; // 새 문자가 없지만 버퍼에 입력이 남아 있으면 입력 정지 시간을 증가시킵니다.

        if(uart_input_wait >= UART_INPUT_TIMEOUT) {
            if(timer_parse_input(uart_input, uart_input_len, timer_seconds)) {
                updated = 1; // 엔터 없이도 일정 시간이 지나 정상 입력이면 타이머 값을 갱신합니다.
            }

            uart_input_len = 0; // 자동 파싱 후 입력 버퍼를 비웁니다.
            uart_input_wait = 0; // 자동 파싱 후 대기 카운터를 초기화합니다.
        }
    }

    return updated; // 타이머 값이 갱신되었으면 1, 아니면 0을 반환합니다.
}

void timer_display_update(uint16_t timer_seconds)
{
    uint8_t min = timer_seconds / 60; // 전체 초를 60으로 나누어 FND 앞 두 자리에 표시할 분 값을 구합니다.
    uint8_t sec = timer_seconds % 60; // 전체 초를 60으로 나눈 나머지로 FND 뒤 두 자리에 표시할 초 값을 구합니다.

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        fnd_display_digits[0] = min / 10; // 첫 번째 FND 자리에 표시할 분의 십의 자리 숫자를 저장합니다.
        fnd_display_digits[1] = min % 10; // 두 번째 FND 자리에 표시할 분의 일의 자리 숫자를 저장합니다.
        fnd_display_digits[2] = sec / 10; // 세 번째 FND 자리에 표시할 초의 십의 자리 숫자를 저장합니다.
        fnd_display_digits[3] = sec % 10; // 네 번째 FND 자리에 표시할 초의 일의 자리 숫자를 저장합니다.
    }
}