#include <stdio.h>
#include <stdint.h>

/*
 * 정답 2 — 세트된 비트 개수 세기 (popcount)
 *
 * 핵심 포인트:
 *  - 8개 비트를 차례로 검사해 1 인 개수를 센다: (v >> i) & 1.
 *  - 레지스터에서 "켜진 플래그가 몇 개인가"를 셀 때 쓰는 기본 패턴이다.
 *
 * 흔한 실수:
 *  - (v & (1<<i)) 결과(0 또는 1<<i)를 그대로 더해 개수가 아니라 값을 더하는 경우.
 *    1 인지 여부만 세려면 >> 로 LSB 까지 내리거나 != 0 으로 비교한다.
 */

static int count_set_bits(uint8_t v)
{
    int i;
    int count = 0;
    for (i = 0; i < 8; i++) {
        count += (int)((v >> i) & 1u);
    }
    return count;
}

int main(void)
{
    uint8_t values[] = { 0x00, 0x01, 0x0F, 0xAA, 0xFF };
    int n = (int)(sizeof(values) / sizeof(values[0]));
    int k;

    for (k = 0; k < n; k++) {
        printf("0x%02X has %d set bits\n",
               values[k], count_set_bits(values[k]));
    }

    return 0;
}
