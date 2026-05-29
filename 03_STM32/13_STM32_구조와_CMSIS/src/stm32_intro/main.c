#include "stm32f10x.h"

/*
 * 13강 예제 — CMSIS 레지스터 접근의 기본형.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM (Arm Compiler 6).
 *
 * 이 편은 '구조 이해'가 목표다. 실제 GPIO 출력으로 LED 를 켜는 일은 14편에서
 * 한다. 여기서는 CMSIS 헤더로 주변장치 클럭을 켜는 표준 관용구를 보여 주고,
 * 그 결과(RCC 레지스터 비트)가 시뮬레이터 레지스터 창에서 바뀌는 것을 관찰한다.
 *
 * STM32 의 철칙: "주변장치를 쓰기 전에 먼저 그 주변장치의 클럭을 켠다."
 * 클럭이 꺼진 주변장치는 레지스터에 값을 써도 반응하지 않는다.
 */

int main(void)
{
    volatile uint32_t heartbeat = 0;   /* 살아 있음을 보여 주는 카운터 */

    /* GPIOA, GPIOC 는 APB2 버스에 매달려 있다. 두 포트의 클럭을 켠다.
     * 켜기 전에는 GPIOA->/GPIOC-> 레지스터가 동작하지 않는다. */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    /* SystemCoreClock 은 CMSIS 가 관리하는 현재 코어 클럭(Hz) 전역 변수다.
     * Watch 창에 추가하면 system_stm32f10x.c 설정값(보통 72000000)이 보인다. */

    while (1) {
        heartbeat++;       /* Watch 창에서 값이 증가하는 것을 관찰 */
    }
}
