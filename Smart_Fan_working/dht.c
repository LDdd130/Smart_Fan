#include "dht.h"

// DHT DATA 선을 입력 모드로 전환한다.
// 시작 신호 이후 센서가 DATA 선을 구동하므로 MCU 핀은 반드시 입력으로 바뀌어야 한다.
void DHT_SetInput(void)
{
    // DHT 통신은 1-wire 형태라 MCU가 LOW 시작 신호를 보낸 뒤,
    // 같은 DATA 핀을 입력으로 바꿔 센서의 응답 펄스를 읽는다.
    DHT_DDR &= ~(1 << DHT_DATA);

    // 외부 풀업 저항이 없다면 내부 풀업으로 DATA 라인을 HIGH idle 상태에 둔다.
    // DHT 모듈에 이미 풀업이 있다면 이 설정은 보조 역할만 한다.
    DHT_PORT |= (1 << DHT_DATA); // DHT DATA 라인은 idle 상태에서 HIGH가 필요하다.
}

// DHT DATA 선을 출력 LOW로 만든다.
// DHT 읽기 시작 조건을 만들 때만 사용하며, 이 상태를 오래 유지하면 센서 통신이 막힌다.
static void DHT_SetOutputLow(void)
{
    // DHT 읽기를 시작하려면 MCU가 DATA 라인을 최소 18ms 동안 LOW로 잡아야 한다.
    DHT_DDR |= (1 << DHT_DATA);
    DHT_PORT &= ~(1 << DHT_DATA);
}

// DHT DATA 선이 원하는 상태가 될 때까지 기다린다.
// 반환값: 1 = 원하는 상태 감지 성공, 0 = timeout 발생
static uint8_t DHT_WaitForState(uint8_t state, uint16_t timeout_us)
{
    // DATA 핀이 원하는 상태(state: 0=LOW, 1=HIGH)가 될 때까지 1us 단위로 기다린다.
    // 센서가 응답하지 않거나 배선 문제가 있을 때 무한 대기하지 않도록 timeout을 둔다.
    while (((DHT_PIN & (1 << DHT_DATA)) ? 1 : 0) != state) {
        if (timeout_us == 0) {
            return 0;
        }

        timeout_us--;
        _delay_us(1);
    }

    return 1;
}

// DHT 센서에서 온도/습도를 1회 읽는다.
// 반환값: 1 = 읽기 성공 및 checksum 통과, 0 = 응답 없음/타이밍 오류/checksum 오류
uint8_t DHT_Read(uint8_t *temperature, uint8_t *humidity)
{
    // DHT11 기준 데이터 형식:
    // data[0] = 습도 정수부
    // data[1] = 습도 소수부
    // data[2] = 온도 정수부
    // data[3] = 온도 소수부
    // data[4] = checksum(data[0]~data[3] 합의 하위 8비트)
    uint8_t data[5] = { 0, };

    // 1. MCU 시작 신호: DATA를 출력 LOW로 만들고 18ms 유지한다.
    DHT_SetOutputLow();
    _delay_ms(18);

    // 2. MCU가 DATA를 입력으로 전환한다. 이후부터 센서가 라인을 구동한다.
    DHT_SetInput();
    _delay_us(30);

    // 3. 센서 응답 확인:
    //    센서가 LOW 약 80us -> HIGH 약 80us -> LOW 순서로 응답하면 통신 시작 가능.
    if (!DHT_WaitForState(0, 100)) return 0;
    if (!DHT_WaitForState(1, 100)) return 0;
    if (!DHT_WaitForState(0, 100)) return 0;

    // 4. 총 40비트 수신.
    //    각 비트는 LOW 구간 뒤 HIGH 구간 길이로 0/1을 구분한다.
    //    이 구현은 HIGH 시작 후 40us 지점에서 DATA가 HIGH이면 1, LOW이면 0으로 판정한다.
    for (uint8_t i = 0; i < 40; i++) {
        // 각 비트의 HIGH 구간이 시작될 때까지 기다린다.
        if (!DHT_WaitForState(1, 100)) return 0;

        // DHT11은 0이면 HIGH가 약 26~28us, 1이면 약 70us 유지된다.
        // 40us 뒤에도 HIGH라면 1로 판단한다.
        _delay_us(40);

        if (DHT_PIN & (1 << DHT_DATA)) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }

        // 다음 비트로 넘어가기 위해 현재 HIGH 구간이 끝나 LOW가 될 때까지 기다린다.
        if (!DHT_WaitForState(0, 100)) return 0;
    }

    // 5. 체크섬 검증. 합이 맞지 않으면 노이즈/타이밍 오류로 보고 실패 처리한다.
    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return 0;
    }

    // 6. 현재 코드는 DHT11 정수부 값만 사용한다.
    *humidity = data[0];
    *temperature = data[2];

    return 1;
}