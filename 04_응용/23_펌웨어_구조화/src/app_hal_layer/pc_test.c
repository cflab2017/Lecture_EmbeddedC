#include "app.h"
#include <stdint.h>

/*
 * 23강 PC 검산 — 같은 app_blink_pattern 을 PC HAL(hal_pc.c)로 구동.
 * Keil 에서는 hal_pc.c 대신 hal_stm32.c 를 넣으면 동일 코드가 실보드에서 동작.
 * 빌드: gcc pc_test.c app.c hal_pc.c -o pc_test
 */

int main(void)
{
    uint8_t pattern[6] = { 1, 0, 1, 1, 0, 0 };
    app_blink_pattern(pattern, 6u);
    return 0;
}
