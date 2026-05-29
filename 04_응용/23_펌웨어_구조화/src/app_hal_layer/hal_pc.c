#include "hal.h"
#include <stdio.h>

/*
 * HAL 의 PC 구현. 하드웨어 대신 printf 로 동작을 보여 준다.
 * 응용 로직을 보드 없이 단위 테스트할 때 쓰는 '가짜(fake)' 드라이버.
 */
void hal_led_init(void)
{
    printf("[HAL-PC] led init\n");
}

void hal_led_set(uint8_t on)
{
    printf("[HAL-PC] LED %s\n", on ? "ON" : "OFF");
}
