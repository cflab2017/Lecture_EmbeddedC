#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 비트 순서 뒤집기 함수
 *
 * 핵심 포인트:
 *  - 로직을 reverse_bits() 함수로 분리하면 main 은 '무엇을' 하는지만 보면 된다.
 *  - i 번 비트를 읽어 반대쪽 (7 - i) 위치에 놓는다.
 *
 * 흔한 실수:
 *  - 결과 누적 변수에 0 초기화를 빠뜨리는 경우.
 *  - 비트를 같은 위치에 다시 넣어(뒤집지 않아) 입력과 같은 값이 나오는 경우.
 */

static uint8_t reverse_bits(uint8_t v)
{
    uint8_t out = 0;
    int i;
    for (i = 0; i < 8; i++) {
        if ((v >> i) & 1u) {
            out = (uint8_t)(out | (1u << (7 - i)));
        }
    }
    return out;
}

int main(void)
{
    uint8_t values[] = { 0x01, 0x0F, 0x12, 0x80 };
    int n = (int)(sizeof(values) / sizeof(values[0]));
    int k;

    for (k = 0; k < n; k++) {
        printf("reverse(0x%02X) = 0x%02X\n",
               values[k], reverse_bits(values[k]));
    }

    return 0;
}
