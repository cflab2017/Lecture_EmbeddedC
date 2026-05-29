#include "stm32f10x.h"

/*
 * 18강 예제 — EXTI0(PA0) 외부 인터럽트로 LED 토글.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 버튼을 PA0 와 VCC 사이에 달고 내부 풀다운을 켠다(평소 0, 누르면 1).
 * 누르는 순간의 '상승 에지'를 EXTI 라인0 이 잡아 인터럽트를 건다.
 * 메인 루프는 아무 일도 안 하고, 토글은 인터럽트 핸들러가 처리한다.
 */

#define BTN_PIN   0u     /* PA0 / EXTI0 */
#define LED_PIN   13u    /* PC13 */

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0) {        /* 라인0 펜딩? */
        EXTI->PR = EXTI_PR_PR0;          /* 1 을 써서 펜딩 클리어 */
        GPIOC->ODR ^= (1u << LED_PIN);   /* LED 토글 */
    }
}

int main(void)
{
    /* 클럭: GPIOA, GPIOC, 그리고 EXTI 매핑에 필요한 AFIO */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PA0: 입력 + 풀다운 (버튼은 PA0~VCC, 누르면 상승 에지) */
    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));
    GPIOA->ODR &= ~(1u << BTN_PIN);                   /* ODR=0 → 풀다운 */

    /* PC13: 푸시풀 출력 2MHz */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    /* EXTI0 라인을 PA0 에 연결 (EXTICR1 의 EXTI0 필드 = 0 → 포트 A) */
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;

    /* 라인0: 상승 에지 트리거 + 인터럽트 마스크 해제 */
    EXTI->RTSR |= EXTI_RTSR_TR0;
    EXTI->IMR  |= EXTI_IMR_MR0;

    /* NVIC 에서 EXTI0 인터럽트 활성화 */
    NVIC_EnableIRQ(EXTI0_IRQn);

    while (1) {
        /* 메인은 한가하다 — 버튼 처리는 인터럽트가 한다 */
    }
}
