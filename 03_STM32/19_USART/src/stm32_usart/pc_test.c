#include <stdio.h>
#include <stdint.h>

/*
 * 19강 PC 검산 — USART BRR(보율 분주값) 계산.
 * BRR = fCK / baud (USART1 은 PCLK2 = 72MHz).
 * 정수로 자르면 실제 보율은 fCK/BRR 이라 약간 오차가 생길 수 있다.
 * (72MHz 는 흔한 보율로 잘 나눠떨어져 오차가 0 이다.)
 */

static void report(uint32_t fck, uint32_t baud)
{
    uint32_t brr    = fck / baud;
    uint32_t actual = fck / brr;
    printf("baud %lu -> BRR=%lu (actual %lu)\n",
           (unsigned long)baud, (unsigned long)brr, (unsigned long)actual);
}

int main(void)
{
    uint32_t fck = 72000000u;
    report(fck, 9600u);
    report(fck, 19200u);
    report(fck, 38400u);
    report(fck, 57600u);
    report(fck, 115200u);
    return 0;
}
