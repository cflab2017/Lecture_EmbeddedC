#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 포인터로 가짜 레지스터 조작
 *
 * 핵심 포인트:
 *  - 레지스터를 포인터로 다룬다: *reg 가 곧 그 주소의 값이다.
 *  - 비트 조작 관용구(set/clear/toggle)는 *reg 에 그대로 적용된다.
 *
 * 흔한 실수:
 *  - reg(주소)와 *reg(값)을 혼동하는 경우. 비트를 켜는 대상은 *reg 다.
 *  - 16비트 레지스터인데 %04X 의 0 패딩을 빠뜨려 자리수가 흔들리는 경우.
 */

#define BIT(n) (1u << (n))

int main(void)
{
    uint16_t fake_reg = 0x0000;        /* 16비트 가짜 레지스터 */
    volatile uint16_t *reg = &fake_reg;
    int step = 0;

    *reg |= BIT(0);                    /* set bit0  */
    printf("step %d: 0x%04X\n", ++step, (unsigned)*reg);

    *reg |= BIT(12);                   /* set bit12 */
    printf("step %d: 0x%04X\n", ++step, (unsigned)*reg);

    *reg ^= BIT(0);                    /* toggle bit0 */
    printf("step %d: 0x%04X\n", ++step, (unsigned)*reg);

    *reg |= BIT(4);                    /* set bit4  */
    printf("step %d: 0x%04X\n", ++step, (unsigned)*reg);

    return 0;
}
