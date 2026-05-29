#include <stdio.h>
#include <stdint.h>

/*
 * 13강 PC 검산 — STM32F103 클럭 트리를 손으로 계산한다.
 *
 *   HSE(외부 8MHz) → PLL(×배수) → SYSCLK → AHB(/HPRE)  → HCLK
 *                                          → APB1(/PPRE1) → PCLK1
 *                                          → APB2(/PPRE2) → PCLK2
 *
 * 하드웨어 없이 설정값만으로 각 버스 주파수를 구한다.
 * (F103 제약: SYSCLK/HCLK ≤ 72MHz, PCLK1 ≤ 36MHz, PCLK2 ≤ 72MHz)
 */

typedef struct {
    uint32_t hse_hz;     /* 외부 크리스털 주파수 */
    uint32_t pll_mul;    /* PLL 곱셈 배수 (2~16) */
    uint32_t ahb_div;    /* AHB 프리스케일러 (1,2,...,512) */
    uint32_t apb1_div;   /* APB1 프리스케일러 */
    uint32_t apb2_div;   /* APB2 프리스케일러 */
} clock_cfg_t;

static void report(const char *name, clock_cfg_t c)
{
    uint32_t sysclk = c.hse_hz * c.pll_mul;
    uint32_t hclk   = sysclk / c.ahb_div;
    uint32_t pclk1  = hclk / c.apb1_div;
    uint32_t pclk2  = hclk / c.apb2_div;

    printf("[%s]\n", name);
    printf("  SYSCLK = %lu Hz\n", (unsigned long)sysclk);
    printf("  HCLK   = %lu Hz\n", (unsigned long)hclk);
    printf("  PCLK1  = %lu Hz\n", (unsigned long)pclk1);
    printf("  PCLK2  = %lu Hz\n", (unsigned long)pclk2);
}

int main(void)
{
    /* 표준 72MHz 설정: HSE 8MHz × 9 = 72MHz, AHB/1, APB1/2, APB2/1 */
    clock_cfg_t std72 = { 8000000u, 9u, 1u, 2u, 1u };
    /* 다른 예: HSE 8MHz × 4 = 32MHz, AHB/1, APB1/1, APB2/1 */
    clock_cfg_t cfg32 = { 8000000u, 4u, 1u, 1u, 1u };

    report("72MHz standard", std72);
    report("32MHz example", cfg32);

    return 0;
}
