#include "button.h"

void ButtonInit(BUTTON *button, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum)
{
    button->ddr = ddr;   // 버튼이 연결된 포트의 주소를 저장
    button->pin = pin;   // 버튼이 연결된 핀 번호를 저장
    button->btnPin = pinNum; // 버튼 핀 번호를 저장
    button->prevState = RELEASED; // 초기 상태는 RELEASED로 설정
    button->debounceCount = 0;

    *(button->ddr) &= ~(1 << button->btnPin); // 버튼 핀을 입력으로 설정
}

uint8_t ButtonGetState(BUTTON *button)
{
    uint8_t curState = ((*(button->pin) & (1 << button->btnPin)) == 0) ? PUSHED : RELEASED;

    if (curState == button->prevState)
    {
        button->debounceCount = 0;
        return NO_ACT;
    }

    if (button->debounceCount < BUTTON_DEBOUNCE_COUNT)
    {
        button->debounceCount++;
        return NO_ACT;
    }

    button->debounceCount = 0;
    button->prevState = curState;

    if (curState == PUSHED)
    {
        return ACT_PUSH;
    }

    return ACT_RELEASE;
}