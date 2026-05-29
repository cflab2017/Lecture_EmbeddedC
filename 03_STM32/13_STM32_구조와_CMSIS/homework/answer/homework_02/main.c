#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — GPIOB와 TIM2 클럭 켜기
 *
 * 핵심 포인트:
 *  - 주변장치마다 매달린 버스가 달라 인에이블 레지스터도 다르다.
 *      GPIOB → 고속 APB2 → RCC->APB2ENR 의 IOPBEN
 *      TIM2  → 저속 APB1 → RCC->APB1ENR 의 TIM2EN
 *  - 비트 이름은 stm32f10x.h 의 매크로를 써서 매직 넘버를 피한다.
 *
 * 흔한 실수:
 *  - GPIOB 를 APB1 로, TIM2 를 APB2 로 착각하는 경우(버스가 반대다).
 *  - 클럭을 켜지 않고 GPIOB/TIM2 레지스터를 건드려 아무 반응이 없는 경우.
 *
 * 예상 동작(시뮬레이터): RCC->APB2ENR 의 IOPBEN(비트3)=1(0x08),
 *  RCC->APB1ENR 의 TIM2EN(비트0)=1(0x01) 이 레지스터 창에서 확인된다.
 */

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;   /* GPIOB 클럭 (APB2) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   /* TIM2 클럭 (APB1) */

    while (1) {
        /* 이 편은 클럭 인에이블 확인까지. 실제 GPIO/타이머 동작은 다음 편들. */
    }
}
