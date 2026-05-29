#include <stdio.h>

/*
 * 12강 PC 검산 — 0~9 의 공통 음극 세그먼트 코드와 켜지는 세그먼트를 출력한다.
 * 하드웨어 없이 룩업 테이블의 정확성만 확인한다.
 */

static const unsigned char SEG_CODE[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

/* 켜진 세그먼트는 a~g 글자로, 꺼진 세그먼트는 '-' 로 표시한다. */
static void print_seg(unsigned char code)
{
    static const char names[7] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
    int i;
    for (i = 0; i < 7; i++) {
        putchar((code & (1u << i)) ? names[i] : '-');
    }
}

int main(void)
{
    int n;
    for (n = 0; n <= 9; n++) {
        printf("%d -> 0x%02X  ", n, SEG_CODE[n]);
        print_seg(SEG_CODE[n]);
        putchar('\n');
    }
    return 0;
}
