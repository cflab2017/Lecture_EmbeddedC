#include <stdio.h>
#include <stdint.h>

/*
 * 16강 PC 검산 — 부호 없는 틱 카운터의 경과 시간 계산.
 * elapsed = now - start (uint32 래핑 산술). 카운터가 한 바퀴 돌아도
 * 빼기 결과가 올바른 경과 시간을 준다(실제 경과가 2^32 이내일 때).
 * 그래서 delay_ms 의 (g_ms - start) < ms 가 래핑에도 안전하다.
 */

static uint32_t elapsed(uint32_t now, uint32_t start)
{
    return now - start;     /* 부호 없는 뺄셈 → 래핑 자동 처리 */
}

int main(void)
{
    /* {start, now} 쌍: 마지막 둘은 카운터가 한 바퀴 돈(래핑) 경우 */
    uint32_t s[4] = { 1000u, 0u, 0xFFFFFFFFu, 0xFFFFFF00u };
    uint32_t n[4] = { 1500u, 10u, 0x00000004u, 0x00000100u };
    int i;

    for (i = 0; i < 4; i++) {
        printf("start=%lu now=%lu -> elapsed=%lu\n",
               (unsigned long)s[i], (unsigned long)n[i],
               (unsigned long)elapsed(n[i], s[i]));
    }
    return 0;
}
