#include <reg52.h>

/*
 * 07강 예제 1 — 흐르는 불빛(running light).
 * 대상: AT89C52, 컴파일러: Keil C51.
 *
 * P1 에 LED 8개가 연결돼 있다고 가정한다. 많은 8051 보드는 LED 를
 * '액티브 로우'로 단다(핀이 0 일 때 LED ON). 그래서 한 LED 만 켜려면
 * 그 비트만 0, 나머지는 1 인 값을 포트에 쓴다: ~(1 << pos).
 */

#define LED_PORT  P1

/* 소프트웨어 지연(정밀하지 않음). 시뮬레이터 관찰용으로 짧게 잡는다.
 * 정확한 시간 지연은 9편(타이머)에서 다룬다. */
static void delay(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) {
        /* 그냥 시간 보내기 */
    }
}

void main(void)
{
    unsigned char pos = 0;

    while (1) {
        /* 액티브 로우: pos 번 비트만 0 → 그 LED 하나만 ON */
        LED_PORT = (unsigned char)~(1u << pos);
        delay(20000);

        pos++;
        if (pos >= 8) {
            pos = 0;          /* 끝에 닿으면 처음 위치로 되돌아간다 */
        }
    }
}
