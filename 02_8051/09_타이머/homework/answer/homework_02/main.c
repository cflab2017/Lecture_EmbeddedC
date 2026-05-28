#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — 긴 지연에 필요한 오버플로 횟수 계산
 *
 * 핵심 포인트:
 *  - 16비트 타이머는 한 번에 최대 ~65ms(1us 단위) 까지만 셀 수 있다.
 *    더 긴 지연은 '오버플로를 여러 번' 세서 만든다.
 *  - 오버플로 1회를 50ms 로 잡으면, 목표 시간 / 50ms = 필요한 오버플로 수.
 *
 * 흔한 실수:
 *  - ms 와 us 단위를 섞는 경우. 여기선 모두 us 로 환산해 나눈다.
 */
int main(void)
{
    unsigned long target_ms[] = { 100, 500, 1000 };
    int n = (int)(sizeof(target_ms) / sizeof(target_ms[0]));
    unsigned long per_overflow_us = 50000UL;   /* 오버플로 1회 = 50ms */
    int k;

    for (k = 0; k < n; k++) {
        unsigned long total_us = target_ms[k] * 1000UL;
        unsigned long overflows = total_us / per_overflow_us;
        printf("%4lu ms -> %2lu overflows (50ms each)\n",
               target_ms[k], overflows);
    }

    return 0;
}
