#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/* 순수 로직: 레지스터의 n 번 비트를 set/clear/toggle/test 한다.
 * 하드웨어 의존이 없어 PC 에서 그대로 검증할 수 있다. */
static uint8_t set_bit(uint8_t reg, unsigned n)    { return (uint8_t)(reg | BIT(n)); }
static uint8_t clear_bit(uint8_t reg, unsigned n)  { return (uint8_t)(reg & ~BIT(n)); }
static uint8_t toggle_bit(uint8_t reg, unsigned n) { return (uint8_t)(reg ^ BIT(n)); }
static int     test_bit(uint8_t reg, unsigned n)   { return (int)((reg >> n) & 1u); }

/*
 * 03강 예제 2 — set/clear/toggle/test 관용구를 함수로 떼어내 검증한다.
 */
int main(void)
{
    uint8_t reg = 0x00;

    reg = set_bit(reg, 0);          /* 0000 0001 */
    reg = set_bit(reg, 7);          /* 1000 0001 */
    printf("after set 0,7  = 0x%02X\n", reg);

    reg = clear_bit(reg, 0);        /* 1000 0000 */
    printf("after clear 0  = 0x%02X\n", reg);

    reg = toggle_bit(reg, 1);       /* 1000 0010 */
    printf("after toggle 1 = 0x%02X\n", reg);

    printf("bit7 = %d, bit6 = %d\n", test_bit(reg, 7), test_bit(reg, 6));

    return 0;
}
