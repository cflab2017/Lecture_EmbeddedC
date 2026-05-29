#include "ledbank.h"

/*
 * ledbank.c — ledbank.h 가 선언한 함수들의 '구현'.
 * 비트 조작 관용구(3편)를 의미 있는 이름의 함수로 감쌌다.
 */

uint8_t led_set(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank | BIT(pos));
}

uint8_t led_clear(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank & ~BIT(pos));
}

uint8_t led_toggle(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank ^ BIT(pos));
}

uint8_t led_count_on(uint8_t bank)
{
    uint8_t pos;
    uint8_t count = 0;
    for (pos = 0; pos < LED_COUNT; pos++) {
        count = (uint8_t)(count + ((bank >> pos) & 1u));
    }
    return count;
}
