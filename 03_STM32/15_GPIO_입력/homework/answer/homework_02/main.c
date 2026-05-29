#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — 풀다운 입력(액티브 하이) 버튼
 *
 * 핵심 포인트:
 *  - 입력 + 풀업/풀다운 = CNF 10, MODE 00 → 0x8. 풀다운은 ODR 비트=0 으로 선택.
 *  - 버튼을 PA1 과 VCC 사이에 달면 눌릴 때 IDR 비트=1(액티브 하이).
 *
 * 흔한 실수:
 *  - 풀다운인데 ODR 을 1 로 둬서 풀업이 되는 경우.
 *  - 액티브 로우(=0 눌림)와 헷갈려 판정을 반대로 하는 경우.
 *
 * 예상 동작(시뮬레이터): GPIOA->CRL 의 PA1 자리=0x8, ODR 비트1=0.
 *  PA1=1 이면 PC13=0(LED ON), PA1=0 이면 PC13=1(OFF).
 */

#define BTN_PIN   1u     /* PA1 */
#define LED_PIN   13u    /* PC13 */

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));   /* 입력 + 풀업/풀다운 */
    GPIOA->ODR &= ~(1u << BTN_PIN);            /* ODR=0 → 풀다운 */

    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    while (1) {
        if (GPIOA->IDR & (1u << BTN_PIN)) {    /* 풀다운: 눌리면 1 */
            GPIOC->BRR  = (1u << LED_PIN);     /* LED ON (액티브 로우) */
        } else {
            GPIOC->BSRR = (1u << LED_PIN);     /* LED OFF */
        }
    }
}
