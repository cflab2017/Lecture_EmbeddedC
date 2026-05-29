#include <stdio.h>
#include "ledbank.h"

/*
 * 05강 예제 2 — ledbank 모듈 함수들을 단위 검증한다.
 * 하드웨어 없이 PC 에서 모듈만 떼어 테스트할 수 있다.
 * 빌드: gcc pc_test.c ledbank.c -o pc_test
 */
int main(void)
{
    uint8_t b = 0x00;

    b = led_set(b, 0);
    b = led_set(b, 3);
    printf("set 0,3   = 0x%02X\n", b);     /* 0x09 */

    b = led_clear(b, 0);
    printf("clear 0   = 0x%02X\n", b);     /* 0x08 */

    b = led_toggle(b, 7);
    printf("toggle 7  = 0x%02X\n", b);     /* 0x88 */

    printf("count on  = %u\n", (unsigned)led_count_on(b));  /* 2 */

    return 0;
}
