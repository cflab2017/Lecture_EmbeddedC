#include "hal.h"
#include <avr/io.h>

/*
 * HAL 의 Arduino Uno(ATmega328P) 구현 — AVR 레지스터 직접 제어.
 *
 * 우노 온보드 LED 는 '디지털 13번' = PB5(포트 B, 비트 5), 액티브 하이(1=켜짐).
 * digitalWrite 같은 Arduino API 를 쓰지 않고 DDRB/PORTB 레지스터를 직접 다룬다
 * — 이 트랙의 '레지스터 직접 제어' 철학을 AVR 에서도 그대로 적용.
 *
 * 빌드: avr-gcc 또는 Arduino IDE(AVR 툴체인). Keil 로는 빌드 불가(AVR 미지원).
 *   app.c 는 한 줄도 바뀌지 않는다 — HAL 구현만 이 파일로 교체하면 우노에서 동작.
 */

#define LED_BIT   5u    /* PB5 = 디지털 13 */

void hal_led_init(void)
{
    DDRB |= (uint8_t)(1u << LED_BIT);     /* PB5 를 출력으로 */
}

void hal_led_set(uint8_t on)
{
    if (on) {
        PORTB |= (uint8_t)(1u << LED_BIT);    /* 액티브 하이: ON = 1 */
    } else {
        PORTB &= (uint8_t)~(1u << LED_BIT);   /* OFF = 0 */
    }
}
