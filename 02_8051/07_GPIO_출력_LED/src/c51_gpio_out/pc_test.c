#include <stdio.h>

/*
 * 07강 예제 2 — 흐르는 불빛이 만들 포트 값 시퀀스를 PC 에서 검산한다.
 * 액티브 로우라 'pos 번 LED ON' = 그 비트만 0 = ~(1 << pos).
 */

static void print_bin8(unsigned char v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

int main(void)
{
    unsigned char pos;

    for (pos = 0; pos < 8; pos++) {
        unsigned char port = (unsigned char)~(1u << pos);
        printf("pos %u: ", (unsigned)pos);
        print_bin8(port);
        printf(" (0x%02X)\n", port);
    }

    return 0;
}
