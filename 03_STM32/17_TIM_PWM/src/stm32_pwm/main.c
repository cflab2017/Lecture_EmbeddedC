#include "stm32f10x.h"

/*
 * 17강 예제 — TIM2 CH1(PA0) PWM 출력.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 타이머 클럭(여기선 72MHz)을 PSC 로 1MHz 로 나누고, ARR 로 주기를,
 * CCR1 로 듀티를 정한다. PWM 모드1: CNT < CCR1 동안 출력 High.
 *   - PSC=71  → 72MHz/(71+1) = 1MHz 카운터
 *   - ARR=999 → 1MHz/(999+1) = 1kHz PWM
 *   - CCR1=250 → 듀티 25%
 */

int main(void)
{
    /* 1) 클럭: GPIOA 는 APB2, TIM2 는 APB1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* 2) PA0 = TIM2_CH1 → 대체 기능 푸시풀 50MHz (CNF=10, MODE=11 → 0xB) */
    GPIOA->CRL &= ~(0xFu << (0u * 4u));
    GPIOA->CRL |=  (0xBu << (0u * 4u));

    /* 3) 타이머 시간축 */
    TIM2->PSC  = 71u;        /* 72MHz / 72 = 1MHz 카운터 */
    TIM2->ARR  = 999u;       /* 1MHz / 1000 = 1kHz PWM */
    TIM2->CCR1 = 250u;       /* 듀티 25% */

    /* 4) 채널1 PWM 모드1(110) + CCR 프리로드, 출력 인에이블 */
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;  /* OC1M = 110 */
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;                      /* CCR1 프리로드 */
    TIM2->CCER  |= TIM_CCER_CC1E;                        /* CH1 출력 켜기 */

    /* 5) ARR 프리로드 + 업데이트 강제 로드(UG) + 카운터 시작 */
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    while (1) {
        /* PWM 은 하드웨어가 자동 생성. CCR1 을 바꾸면 듀티가 즉시 바뀐다. */
    }
}
