#include "stm32f10x.h"

/*
 * 15강 예제 — 버튼 입력으로 LED 제어.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * PA0 를 '입력 + 내부 풀업'으로 두고, 버튼은 PA0 와 GND 사이에 단다.
 *   - 안 누름: 풀업이 1 로 끌어올림 → IDR 비트 = 1
 *   - 누름:    GND 로 연결 → IDR 비트 = 0   (액티브 로우 버튼)
 * 버튼이 눌리면 PC13 LED 를 켠다(LED 도 액티브 로우라 핀=0 이 ON).
 */

#define BTN_PIN   0u     /* PA0 */
#define LED_PIN   13u    /* PC13 */

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    /* PA0: 입력 + 풀업/풀다운. CNF=10, MODE=00 → 4비트 0x8.
     * 풀업이냐 풀다운이냐는 ODR 비트로 고른다(ODR=1 → 풀업). */
    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));
    GPIOA->ODR |=  (1u << BTN_PIN);                    /* 풀업 선택 */

    /* PC13: 푸시풀 출력 2MHz = 0x2 (CRH) */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    while (1) {
        if ((GPIOA->IDR & (1u << BTN_PIN)) == 0u) {
            GPIOC->BRR  = (1u << LED_PIN);   /* 눌림: LED ON (=0) */
        } else {
            GPIOC->BSRR = (1u << LED_PIN);   /* 안 눌림: LED OFF (=1) */
        }
    }
}
