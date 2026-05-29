#include <stdio.h>

/*
 * 정답 2 — 이식 가능한 map() 유틸리티
 *
 * 핵심 포인트:
 *  - map(x) = (x-in_lo)*(out_hi-out_lo)/(in_hi-in_lo) + out_lo.
 *  - 곱셈을 먼저, 나눗셈을 나중에 해야 정수 절삭 오차가 작다.
 *  - 칩과 무관한 순수 유틸이라 어떤 플랫폼에서도 재사용된다.
 *
 * 흔한 실수:
 *  - 나눗셈을 먼저 해서 0 이 되는 경우. (x-in_lo)/(in_hi-in_lo) 가 먼저면 0.
 *  - 큰 값에서 오버플로 — 필요하면 long 으로 계산.
 */

static long map_val(long x, long in_lo, long in_hi, long out_lo, long out_hi)
{
    return (x - in_lo) * (out_hi - out_lo) / (in_hi - in_lo) + out_lo;
}

int main(void)
{
    int samples[5] = { 0, 1024, 2048, 3072, 4095 };
    int i;

    for (i = 0; i < 5; i++) {
        long p = map_val(samples[i], 0, 4095, 0, 100);
        printf("map %d -> %ld\n", samples[i], p);
    }
    return 0;
}
