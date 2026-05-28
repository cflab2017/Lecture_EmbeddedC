#include <stdio.h>
#include <stdint.h>

/* 8비트 값을 2진수 8자리로 출력한다.
 * 표준 printf 에는 2진 출력 포맷이 없어 비트를 직접 뽑아 찍는다. */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

/*
 * 02강 예제 1 — 자료형의 실체(폭)와 같은 수의 여러 진수 표기를 본다.
 * stdint 의 폭 고정 타입은 어떤 칩(8051/STM32/PC)에서도 크기가 같다.
 */
int main(void)
{
    uint8_t reg = 0x2A;   /* 16진 0x2A = 10진 42 = 2진 00101010 */

    /* stdint 폭 고정 타입의 크기는 이름 그대로다 (어디서나 동일) */
    printf("sizeof(uint8_t)  = %u byte\n", (unsigned)sizeof(uint8_t));
    printf("sizeof(uint16_t) = %u byte\n", (unsigned)sizeof(uint16_t));
    printf("sizeof(uint32_t) = %u byte\n", (unsigned)sizeof(uint32_t));

    /* 같은 값 하나를 10진/16진/8진/2진으로 본다 */
    printf("\n");
    printf("dec = %u\n",     (unsigned)reg);
    printf("hex = 0x%02X\n", reg);
    printf("oct = 0%o\n",    reg);
    printf("bin = ");
    print_bin8(reg);
    printf("\n");

    return 0;
}
