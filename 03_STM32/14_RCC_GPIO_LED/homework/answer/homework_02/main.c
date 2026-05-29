#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — PA5 출력 점멸
 *
 * 핵심 포인트:
 *  - PA5 는 핀0~7 이라 CRL 을 쓴다. 4비트 위치 = 5*4 = 20.
 *  - 푸시풀 출력 2MHz = 0x2. 클리어 후 설정으로 옆 핀을 보존한다.
 *
 * 흔한 실수:
 *  - PA5 를 CRH 에 설정하는 경우(핀8~15 가 CRH, 핀0~7 은 CRL).
 *  - 클럭(IOPAEN) 인에이블을 빠뜨리는 경우.
 *
 * 예상 동작(시뮬레이터): GPIOA->CRL = 0x44244444 (PC13 자리 아님, PA5 자리=0x2),
 *  PA5(PORTA.5) 가 주기적으로 토글.
 */

#define LED_PIN   5u

static void delay(volatile uint32_t count)
{
    while (count--) { }
}

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;                 /* GPIOA 클럭 */

    GPIOA->CRL &= ~(0xFu << (LED_PIN * 4u));            /* PA5 자리 클리어 */
    GPIOA->CRL |=  (0x2u << (LED_PIN * 4u));            /* 푸시풀 출력 2MHz */

    while (1) {
        GPIOA->ODR ^= (1u << LED_PIN);                  /* PA5 토글 */
        delay(200000u);
    }
}
