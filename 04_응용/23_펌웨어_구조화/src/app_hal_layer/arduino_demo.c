/* arduino_demo: app.c + hal.h + hal_arduino.c 와 함께 빌드 (avr-gcc / Arduino).
 * 우노 온보드 LED(D13=PB5)를 패턴대로 점멸한다. 같은 app.c 를 그대로 쓴다.
 * 빌드 예:
 *   avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os \
 *           arduino_demo.c app.c hal_arduino.c -o blink.elf
 */
#define F_CPU 16000000UL          /* 우노 16MHz */
#include <util/delay.h>
#include "app.h"
#include "hal.h"

int main(void)
{
    uint8_t pattern[6] = { 1, 0, 1, 1, 0, 0 };
    while (1) {
        uint8_t i;
        hal_led_init();
        for (i = 0; i < 6; i++) {
            hal_led_set(pattern[i]);
            _delay_ms(300);       /* 사람이 볼 수 있게 */
        }
    }
}
