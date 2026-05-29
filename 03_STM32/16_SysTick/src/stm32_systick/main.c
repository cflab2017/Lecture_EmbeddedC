#include "stm32f10x.h"

/*
 * 16강 예제 — SysTick 으로 정확한 500ms 점멸.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * SysTick 은 Cortex-M 코어에 내장된 24비트 다운 카운터다.
 * SysTick_Config(N) 이 reload=N-1 로 맞추고 인터럽트를 켜며 시작시킨다.
 * SystemCoreClock/1000 을 넣으면 1ms 마다 SysTick_Handler 가 불린다.
 */

#define LED_PIN   13u    /* PC13 */

static volatile uint32_t g_ms = 0;   /* 1ms 마다 증가하는 시스템 틱 */

void SysTick_Handler(void)
{
    g_ms++;
}

/* g_ms 기반의 정확한 밀리초 지연. 부호 없는 뺄셈이라 래핑도 안전. */
static void delay_ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) {
        /* 대기 */
    }
}

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));   /* 푸시풀 출력 2MHz */

    /* 1ms 틱: SystemCoreClock/1000 (예: 72MHz → 72000) */
    SysTick_Config(SystemCoreClock / 1000u);

    while (1) {
        GPIOC->ODR ^= (1u << LED_PIN);   /* PC13 토글 */
        delay_ms(500u);                  /* 정확히 500ms */
    }
}
