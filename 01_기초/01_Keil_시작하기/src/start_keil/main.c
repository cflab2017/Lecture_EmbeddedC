#include <stdio.h>
#include <stdint.h>

/*
 * 01강 예제 — 임베디드 프로그램의 기본 골격을 PC에서 흉내낸다.
 * 실제 보드에서는 led 변수가 GPIO 핀(LED)에 연결되지만,
 * 여기서는 변수로 시뮬레이션하고 printf 로 상태를 출력한다.
 */
int main(void)
{
    uint8_t led = 0;   /* 0 = 꺼짐, 1 = 켜짐 */
    uint8_t tick;

    printf("Hello, Embedded C!\n");

    /*
     * 임베디드 main 은 보통 while(1) 무한 루프로 동작한다.
     * PC에서는 끝없이 돌면 관찰이 어려우므로 5회로 제한했다.
     */
    for (tick = 0; tick < 5; tick++) {
        led = (uint8_t)!led;                 /* LED 토글: 0<->1 */
        printf("tick %u: LED = %s\n",
               (unsigned)tick, led ? "ON" : "OFF");
    }

    return 0;
}
