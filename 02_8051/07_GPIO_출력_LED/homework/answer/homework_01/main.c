#include <reg52.h>

/*
 * 정답 1 (C51, AT89C52) — 두 LED 교대 점멸
 *
 * 핵심 포인트:
 *  - P1.0 과 P1.1 이 항상 반대 상태가 되도록 한쪽을 토글하면 다른 쪽은
 *    그 반대로 맞춘다(액티브 로우 가정).
 *  - while(1) 안에서 토글 → 지연 → 반복.
 *
 * 흔한 실수:
 *  - 지연을 빼먹어 시뮬레이터에서 너무 빨라 관찰이 안 되는 경우.
 *  - 두 LED 를 따로 토글하다 동시에 같은 상태가 되는 경우.
 *
 * 예상 동작(시뮬레이터): Port 1 에서 P1.0 과 P1.1 이 번갈아 ON/OFF.
 *  LED0 ON 일 때 LED1 OFF, 다음 주기엔 반대.
 */

sbit LED0 = P1 ^ 0;
sbit LED1 = P1 ^ 1;

static void delay(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) {
    }
}

void main(void)
{
    bit state = 0;     /* 0 이면 LED0 ON, 1 이면 LED1 ON */

    while (1) {
        /* 액티브 로우: 0 = ON */
        LED0 = state;          /* state=0 → LED0 ON */
        LED1 = (bit)!state;    /* 항상 반대 */
        delay(30000);

        state = (bit)!state;   /* 다음 주기엔 역할 교대 */
    }
}
