#include <stdio.h>
#include <stdint.h>

/*
 * 17강 PC 검산 — PSC/ARR/CCR 로부터 PWM 주파수와 듀티 계산.
 *   counter_clk = tim_clk / (PSC+1)
 *   pwm_freq    = counter_clk / (ARR+1)
 *   duty(%)     = CCR * 100 / (ARR+1)
 */

static void report(uint32_t tim_clk, uint32_t psc, uint32_t arr, uint32_t ccr)
{
    uint32_t counter_clk = tim_clk / (psc + 1u);
    uint32_t pwm_freq    = counter_clk / (arr + 1u);
    uint32_t duty        = (ccr * 100u) / (arr + 1u);

    printf("PSC=%lu ARR=%lu CCR=%lu -> %lu Hz, duty %lu%%\n",
           (unsigned long)psc, (unsigned long)arr, (unsigned long)ccr,
           (unsigned long)pwm_freq, (unsigned long)duty);
}

int main(void)
{
    report(72000000u, 71u, 999u, 250u);   /* 1kHz, 25% */
    report(72000000u, 71u, 999u, 500u);   /* 1kHz, 50% */
    report(72000000u, 71u, 499u, 250u);   /* 2kHz, 50% */
    return 0;
}
