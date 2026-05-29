#include "stm32f10x.h"

/*
 * 14강 예제 — 온보드 LED(PC13) 점멸.
 * 대상: STM32F103C8 (Blue Pill 계열), 컴파일러: Keil MDK-ARM.
 *
 * Blue Pill 보드는 PC13 에 LED 가 액티브 로우로 달려 있다(핀=0 이면 ON).
 * STM32F1 의 GPIO 설정은 CRL/CRH 레지스터로 핀마다 4비트(MODE+CNF)를 쓴다.
 */

#define LED_PIN   13u                 /* PC13 */

static void delay(volatile uint32_t count)
{
    while (count--) { }
}

int main(void)
{
    /* 1) GPIOC 클럭 켜기 (APB2 버스) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* 2) PC13 을 푸시풀 출력(2MHz)으로 설정.
     *    PC13 은 핀 8~15 영역이라 CRH 를 쓴다. 4비트 위치 = (13-8)*4 = 20.
     *    MODE=10(2MHz 출력), CNF=00(범용 푸시풀) → 4비트 값 0b0010 = 0x2. */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));   /* 해당 4비트 클리어 */
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));   /* 0010 설정 */

    while (1) {
        GPIOC->ODR ^= (1u << LED_PIN);   /* PC13 토글 */
        delay(200000u);
    }
}
