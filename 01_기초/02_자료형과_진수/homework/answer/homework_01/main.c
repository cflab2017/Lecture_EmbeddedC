#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 8비트 값 진수표
 *
 * 핵심 포인트:
 *  - 한 값을 10진(%3u)·16진(0x%02X)·2진(8자리)으로 나란히 본다.
 *  - 2진 출력은 표준 printf 에 포맷이 없으므로 비트를 직접 뽑는다:
 *    (v >> i) & 1 로 i 번째 비트를 0/1 로 얻는다.
 *
 * 흔한 실수:
 *  - %02X 의 '0' 패딩을 빠뜨려 0x0F 가 0xF 로 나오는 경우.
 *  - 2진 출력을 LSB 부터(거꾸로) 찍어 자리가 뒤집히는 경우.
 *    최상위 비트(i=7)부터 i=0 까지 내려와야 사람이 읽는 순서가 된다.
 */

/* 8비트 값을 2진수 8자리로 출력한다 (MSB → LSB 순서). */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

int main(void)
{
    /* 진수 감각을 잡기 좋은 대표 값들 */
    uint8_t values[] = { 0, 1, 15, 16, 170, 255 };
    int n = (int)(sizeof(values) / sizeof(values[0]));
    int k;

    for (k = 0; k < n; k++) {
        printf("dec=%3u  hex=0x%02X  bin=",
               (unsigned)values[k], values[k]);
        print_bin8(values[k]);
        printf("\n");
    }

    return 0;
}
