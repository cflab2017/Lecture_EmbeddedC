#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/*
 * 04강 예제 1 — 메모리 맵드 I/O(MMIO) 개념을 PC에서 흉내낸다.
 * 실제 보드에서는 레지스터가 '고정된 주소'에 있고,
 *   #define GPIO_ODR (*(volatile uint32_t *)0x4001080CUL)
 * 처럼 그 주소를 가리키는 포인터를 역참조해 읽고 쓴다.
 * PC 에는 그런 주소가 없으므로 변수 하나를 '가짜 레지스터'로 삼아
 * 그 주소를 포인터로 다룬다. 동작 원리는 동일하다.
 */
int main(void)
{
    volatile uint32_t fake_reg = 0;       /* 실제론 고정 주소의 하드웨어 레지스터 */
    volatile uint32_t *reg = &fake_reg;   /* 그 주소를 가리키는 포인터 */

    *reg = 0x00000000u;                   /* 포인터 역참조로 레지스터에 쓰기 */
    printf("init      = 0x%08X\n", (unsigned)*reg);

    *reg |= BIT(3);                       /* set bit3 */
    printf("set bit3  = 0x%08X\n", (unsigned)*reg);

    *reg |= BIT(17);                      /* set bit17 */
    printf("set bit17 = 0x%08X\n", (unsigned)*reg);

    *reg &= ~BIT(3);                      /* clear bit3 */
    printf("clr bit3  = 0x%08X\n", (unsigned)*reg);

    /* 포인터로 레지스터를 읽어 특정 비트를 검사 */
    printf("bit17 is %s\n", ((*reg >> 17) & 1u) ? "ON" : "OFF");

    return 0;
}
