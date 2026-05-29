#include "hal.h"
#include "stm32f10x.h"

/*
 * HAL 의 STM32F103 구현. PC13 온보드 LED(액티브 로우).
 * 이 파일만 hal_pc.c 대신 Keil 프로젝트에 넣으면, app.c 를 한 줄도 안 고치고
 * 같은 응용 로직이 실제 보드에서 동작한다 — 이것이 이식성이다.
 * (PC gcc 빌드에는 포함하지 않는다. Keil μVision 용.)
 */

#define LED_PIN   13u

void hal_led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));   /* 푸시풀 출력 2MHz */
}

void hal_led_set(uint8_t on)
{
    if (on) {
        GPIOC->BRR  = (1u << LED_PIN);   /* 액티브 로우: ON = 핀 0 */
    } else {
        GPIOC->BSRR = (1u << LED_PIN);   /* OFF = 핀 1 */
    }
}
