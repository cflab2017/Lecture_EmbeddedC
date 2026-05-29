#include <stdio.h>
#include <stdint.h>

/*
 * 20강 PC 검산 — ADC 카운트(0~4095)를 밀리볼트로 환산.
 *   mV = adc * 3300 / 4095   (기준 전압 3.3V, 12비트)
 * 정수 연산이라 곱한 뒤 나눠 오차를 줄인다.
 */

static uint32_t adc_to_mv(uint16_t adc)
{
    return (uint32_t)adc * 3300u / 4095u;
}

int main(void)
{
    uint16_t samples[5] = { 0u, 1024u, 2048u, 3072u, 4095u };
    int i;

    for (i = 0; i < 5; i++) {
        printf("ADC=%u -> %lu mV\n",
               samples[i], (unsigned long)adc_to_mv(samples[i]));
    }
    return 0;
}
