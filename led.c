#include "led.h"

void ledInit(LED *led)
{
    // 포트에 해당하는 핀을 출력으로 설정
    *(led->port - 1) |= (1 << led->pin);
    // DDR 레지스터는 PORT 레지스터보다 1 낮은 위치에 있다
    // *(led->port - 1) 를 이용해서 포트에서 DDR로 접근
    // |= (1 << led->pin) 를 이용해서 지정된 포트를 설정
}

void ledOn(LED *led)
{
    // 해당 핀을 High로 설정해서 LED를 켬
    *(led->port) |= (1 << led->pin);
}

void ledOff(LED *led)
{
    // 해당 핀을 LOW로 설정해서 LED를 끔
    *(led->port) &= ~(1 << led->pin);
}