#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — EXTI1(PA1) 하강 에지 인터럽트
 *
 * 핵심 포인트:
 *  - 버튼을 PA1~GND 에 달고 풀업(ODR=1) → 평소 1, 누르면 0(하강 에지).
 *  - 하강 에지는 FTSR 로 설정. 라인1 이므로 IMR/PR/IRQn 모두 1번을 쓴다.
 *
 * 흔한 실수:
 *  - 라인 번호 불일치(EXTI1 인데 TR0/PR0/EXTI0_IRQn 사용).
 *  - 펜딩(EXTI->PR = EXTI_PR_PR1) 클리어 누락 → 무한 재진입.
 *
 * 예상 동작(시뮬레이터): PA1 하강 에지마다 EXTI1_IRQHandler 진입, PC13 토글.
 */

#define BTN_PIN   1u     /* PA1 / EXTI1 */
#define LED_PIN   13u    /* PC13 */

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR1) {
        EXTI->PR = EXTI_PR_PR1;          /* 펜딩 클리어 */
        GPIOC->ODR ^= (1u << LED_PIN);   /* LED 토글 */
    }
}

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PA1: 입력 + 풀업 (버튼 PA1~GND, 누르면 하강) */
    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));
    GPIOA->ODR |=  (1u << BTN_PIN);                   /* ODR=1 → 풀업 */

    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;           /* EXTI1 ← PA1 */

    EXTI->FTSR |= EXTI_FTSR_TR1;                      /* 하강 에지 */
    EXTI->IMR  |= EXTI_IMR_MR1;                        /* 마스크 해제 */

    NVIC_EnableIRQ(EXTI1_IRQn);

    while (1) {
    }
}
