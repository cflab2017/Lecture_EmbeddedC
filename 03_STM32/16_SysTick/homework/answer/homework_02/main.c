#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — 1초마다 토글 + 초 카운터
 *
 * 핵심 포인트:
 *  - SysTick 1ms 틱을 세어 긴 주기(1000ms)를 만든다. 24비트 한 번으로는
 *    1초를 못 만들지만, 1ms 를 1000번 세면 된다.
 *  - delay_ms 는 부호 없는 뺄셈으로 래핑에도 안전.
 *
 * 흔한 실수:
 *  - g_ms/seconds 에 volatile 을 빠뜨리는 경우.
 *  - SysTick_Handler 이름 오타로 인터럽트가 안 걸리는 경우.
 *
 * 예상 동작(시뮬레이터): g_ms 1ms 증가, seconds 1초마다 +1, PC13 1초마다 토글.
 */

#define LED_PIN   13u

static volatile uint32_t g_ms = 0;

void SysTick_Handler(void)
{
    g_ms++;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) {
    }
}

int main(void)
{
    volatile uint32_t seconds = 0;

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    SysTick_Config(SystemCoreClock / 1000u);   /* 1ms 틱 */

    while (1) {
        delay_ms(1000u);
        GPIOC->ODR ^= (1u << LED_PIN);   /* 1초마다 토글 */
        seconds++;
    }
}
