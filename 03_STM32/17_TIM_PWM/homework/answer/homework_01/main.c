#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 듀티(%)로부터 CCR 계산
 *
 * 핵심 포인트:
 *  - CCR = duty × (ARR+1) / 100. 곱한 뒤 나눠야 정수 오차가 작다.
 *  - ARR=999 → 주기 1000. 듀티 100% 는 CCR=1000(항상 High).
 *
 * 흔한 실수:
 *  - duty/100 을 먼저 계산해 정수에서 0 이 되는 경우(곱셈을 먼저).
 */

int main(void)
{
    unsigned int arr = 999u;
    unsigned int duties[5] = { 0u, 25u, 50u, 75u, 100u };
    int i;

    for (i = 0; i < 5; i++) {
        uint32_t ccr = (uint32_t)duties[i] * (arr + 1u) / 100u;
        printf("duty %u%% -> CCR %lu\n", duties[i], (unsigned long)ccr);
    }
    return 0;
}
