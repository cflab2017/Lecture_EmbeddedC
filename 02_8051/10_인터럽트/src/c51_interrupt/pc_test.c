#include <stdio.h>

/*
 * 10강 예제 2 — ISR 가 하는 일(카운트 증가 + LED 토글)을 PC 에서 검증.
 * 실제로는 인터럽트가 비동기로 발생하지만, 여기서는 '인터럽트 N번 발생'을
 * 순서대로 적용해 최종 상태(press_count, LED)를 확인한다.
 * 액티브 로우라 led 변수 0 = ON, 1 = OFF.
 */
int main(void)
{
    unsigned char led = 1;       /* 시작: OFF (1=OFF) */
    unsigned int count = 0;
    int events = 5;              /* 인터럽트 5번 발생 가정 */
    int e;

    for (e = 1; e <= events; e++) {
        count++;
        led = (unsigned char)!led;
        printf("IRQ %d: count=%u led=%s\n",
               e, count, led ? "OFF" : "ON");
    }

    return 0;
}
