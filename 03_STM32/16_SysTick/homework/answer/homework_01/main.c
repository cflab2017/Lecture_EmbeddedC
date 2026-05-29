#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — SysTick reload 계산과 24비트 한계
 *
 * 핵심 포인트:
 *  - SysTick_Config(N) 은 reload 레지스터에 N-1 을 쓴다.
 *  - 72MHz 에서 ticks = 72000 × period_ms, reload = ticks - 1.
 *  - 카운터가 24비트라 reload 최댓값은 0xFFFFFF(16,777,215). 넘으면 불가.
 *
 * 흔한 실수:
 *  - reload 와 ticks 를 혼동(reload 는 ticks-1).
 *  - 24비트 한계를 32비트로 착각해 1초 주기가 되는 줄 아는 경우.
 */

int main(void)
{
    unsigned int periods[4] = { 1u, 10u, 100u, 1000u };
    int i;

    for (i = 0; i < 4; i++) {
        uint32_t ticks  = 72000u * periods[i];   /* 72MHz 기준 */
        uint32_t reload = ticks - 1u;
        const char *tag = (reload <= 0xFFFFFFu) ? "OK" : "OVERFLOW";
        printf("period %u ms -> reload %lu (%s)\n",
               periods[i], (unsigned long)reload, tag);
    }
    return 0;
}
