#include <stdio.h>
#include <stdint.h>

/* 순수 로직: LED 상태를 뒤집는다. 하드웨어 의존 없음 → PC에서 검증 가능. */
static uint8_t toggle(uint8_t state)
{
    return (uint8_t)!state;
}

int main(void)
{
    uint8_t led = 0;
    uint8_t i;

    for (i = 0; i < 5; i++) {
        led = toggle(led);
        printf("tick %u: LED = %s\n", (unsigned)i, led ? "ON" : "OFF");
    }

    return 0;
}
