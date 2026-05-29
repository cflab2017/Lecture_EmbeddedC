#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — TIM2 CH2(PA1) PWM 1kHz 75%
 *
 * 핵심 포인트:
 *  - 채널2는 CCMR1 의 상위 바이트(OC2M/OC2PE)와 CCER 의 CC2E, 듀티는 CCR2.
 *  - 시간축(PSC/ARR)은 타이머 공통이라 채널1 예제와 같다.
 *
 * 흔한 실수:
 *  - 채널2 인데 OC1M/CC1E/CCR1 을 건드리는 경우(채널 번호 일치 필요).
 *  - PA1 을 AF 푸시풀(0xB)이 아니라 일반 출력으로 두는 경우.
 *
 * 예상 동작(시뮬레이터): PORTA.1 에 1kHz, 75% 듀티 PWM. CCR2=750, CC2E=1.
 */

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PA1 = TIM2_CH2 → AF 푸시풀 (비트[7:4] = 0xB) */
    GPIOA->CRL &= ~(0xFu << (1u * 4u));
    GPIOA->CRL |=  (0xBu << (1u * 4u));

    TIM2->PSC  = 71u;        /* 1MHz 카운터 */
    TIM2->ARR  = 999u;       /* 1kHz */
    TIM2->CCR2 = 750u;       /* 듀티 75% */

    TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;  /* CH2 PWM 모드1 */
    TIM2->CCMR1 |= TIM_CCMR1_OC2PE;                      /* CCR2 프리로드 */
    TIM2->CCER  |= TIM_CCER_CC2E;                        /* CH2 출력 켜기 */

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    while (1) {
    }
}
