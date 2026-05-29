#include <stdio.h>
#include <stdint.h>

/*
 * 18강 PC 검산 — EXTI 인터럽트 처리 로직 흉내.
 * 에지가 발생할 때마다 ISR 이 LED 를 토글하고 카운터를 늘린다.
 * (실제 하드웨어에서는 펜딩 플래그를 클리어해야 다음 인터럽트가 걸린다.)
 */

int main(void)
{
    int events[8] = { 1, 0, 1, 1, 0, 1, 0, 1 };  /* 1 = 에지 발생 */
    uint32_t count = 0;
    int led = 0;
    int i;

    for (i = 0; i < 8; i++) {
        if (events[i]) {            /* 인터럽트 발생 */
            count++;
            led = !led;             /* ISR 동작: LED 토글 */
        }
        printf("t=%d event=%d -> count=%lu LED=%s\n",
               i, events[i], (unsigned long)count, led ? "ON" : "OFF");
    }
    return 0;
}
