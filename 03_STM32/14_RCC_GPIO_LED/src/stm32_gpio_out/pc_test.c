#include <stdio.h>
#include <stdint.h>

/*
 * 14강 PC 검산 — STM32F1 의 CRL/CRH 핀 설정 비트 조작을 흉내 낸다.
 * 핀 하나를 푸시풀 출력(2MHz, 4비트 값 0x2)으로 바꾸는 read-modify-write 를
 * 하드웨어 없이 계산한다. CRH 리셋값은 0x44444444(모든 핀 입력 플로팅).
 */

/* pin(8~15)을 mode4(4비트 설정)로 바꾼 새 CRH 값을 돌려준다. */
static uint32_t set_pin_config(uint32_t crh, unsigned int pin, uint32_t mode4)
{
    unsigned int shift = (pin - 8u) * 4u;
    crh &= ~(0xFu << shift);     /* 기존 4비트 지우고 */
    crh |=  (mode4 << shift);    /* 새 4비트 넣기 */
    return crh;
}

int main(void)
{
    uint32_t crh = 0x44444444u;     /* CRH 리셋값 */

    printf("reset CRH = 0x%08lX\n", (unsigned long)crh);
    crh = set_pin_config(crh, 13u, 0x2u);   /* PC13 = 푸시풀 출력 2MHz */
    printf("after PC13 output = 0x%08lX\n", (unsigned long)crh);

    return 0;
}
