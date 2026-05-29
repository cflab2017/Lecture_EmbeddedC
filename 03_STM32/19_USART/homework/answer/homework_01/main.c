#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 바이트를 16진수 ASCII 두 글자로
 *
 * 핵심 포인트:
 *  - 한 바이트는 상위/하위 니블(4비트) 둘로 나눠 각각 16진 문자로 바꾼다.
 *  - 니블 0~9 는 '0'+n, 10~15 는 'A'+(n-10).
 *  - UART 로 숫자를 사람이 읽게 보낼 때 늘 쓰는 패턴.
 *
 * 흔한 실수:
 *  - 10~15 에 'A' 대신 '0'+n 을 그대로 써서 깨진 문자가 나오는 경우.
 */

static char nibble_to_hex(unsigned int n)
{
    return (n < 10u) ? (char)('0' + n) : (char)('A' + (n - 10u));
}

int main(void)
{
    unsigned char bytes[5] = { 0x00u, 0x2Au, 0xFFu, 0x7Eu, 0xA5u };
    int i;

    for (i = 0; i < 5; i++) {
        unsigned char b = bytes[i];
        char hi = nibble_to_hex((b >> 4) & 0xFu);
        char lo = nibble_to_hex(b & 0xFu);
        printf("0x%02X -> \"%c%c\"\n", b, hi, lo);
    }
    return 0;
}
