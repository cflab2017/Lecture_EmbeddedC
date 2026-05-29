#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — ADC 카운트를 퍼센트(%)로
 *
 * 핵심 포인트:
 *  - pct = adc × 100 / 4095. 곱한 뒤 나눠야 정수 오차가 작다.
 *  - 풀스케일 4095 가 100% 다(2^12 - 1).
 *
 * 흔한 실수:
 *  - adc/4095 를 먼저 해서 0 이 되는 경우(곱셈 먼저).
 *  - 절삭 때문에 3071 이 75 가 아니라 74 인 것을 버그로 오해.
 */

int main(void)
{
    uint16_t samples[5] = { 0u, 1024u, 2048u, 3071u, 4095u };
    int i;

    for (i = 0; i < 5; i++) {
        uint32_t pct = (uint32_t)samples[i] * 100u / 4095u;
        printf("ADC=%u -> %lu%%\n", samples[i], (unsigned long)pct);
    }
    return 0;
}
