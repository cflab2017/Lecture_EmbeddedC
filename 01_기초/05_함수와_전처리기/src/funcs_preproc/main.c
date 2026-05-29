#include <stdio.h>
#include "ledbank.h"   /* 우리 모듈의 인터페이스 */

/* 8비트 값을 2진수 8자리로 출력한다. */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

/*
 * 05강 예제 1 — 모듈(ledbank)을 가져다 쓰는 쪽.
 * main 은 비트 연산의 '방법'을 몰라도, 함수 이름만으로 의도를 표현한다.
 * 빌드: gcc main.c ledbank.c -o main
 */
int main(void)
{
    uint8_t bank = 0x00;

    bank = led_set(bank, 1);
    bank = led_set(bank, 4);
    bank = led_set(bank, 6);
    bank = led_toggle(bank, 4);   /* 4번은 켰다가 다시 끔 */

    printf("bank   = ");
    print_bin8(bank);
    printf("\n");
    printf("LED on = %u\n", (unsigned)led_count_on(bank));

    return 0;
}
