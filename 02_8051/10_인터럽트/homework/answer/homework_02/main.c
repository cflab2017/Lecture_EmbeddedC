#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — 타이머 오버플로를 세어 0.5초마다 LED 토글
 *
 * 핵심 포인트:
 *  - 오버플로 1회 = 50ms. 10회마다 500ms 이므로 그때 LED 를 토글한다.
 *  - 인터럽트가 누적 카운트를 세고, 일정 횟수에서 동작하는 전형적 패턴.
 *
 * 흔한 실수:
 *  - 매 오버플로마다 토글해 주기가 50ms 가 되는 경우(원하는 건 500ms).
 */
int main(void)
{
    unsigned char led = 0;     /* 0=OFF 로 시작 */
    int i;

    for (i = 1; i <= 30; i++) {
        if (i % 10 == 0) {                 /* 10회(=500ms)마다 */
            led = (unsigned char)!led;
            printf("overflow %2d (%4d ms): LED -> %s\n",
                   i, i * 50, led ? "ON" : "OFF");
        }
    }

    return 0;
}
