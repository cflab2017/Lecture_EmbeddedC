#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — ADC 임계값으로 LED 제어
 *
 * 핵심 포인트:
 *  - 채널1(PA1)을 SQR3 첫 자리에 넣고 PA1 을 아날로그 입력(0x0)으로.
 *  - 변환 결과가 절반(2048)보다 크면 PC13 LED ON(액티브 로우=0).
 *
 * 흔한 실수:
 *  - 채널은 1 로 바꿨는데 PA0 을 아날로그로 두는 경우(핀/채널 불일치).
 *  - 액티브 로우라 ON 이 0 임을 잊는 경우.
 *
 * 예상 동작(시뮬레이터): adc_val > 2048 → PC13=0(LED ON), 아니면 PC13=1(OFF).
 */

#define LED_PIN   13u

static void short_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 10000u; i++) { }
}

static void adc1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_ADC1EN;
    RCC->CFGR    |= RCC_CFGR_ADCPRE_DIV6;

    GPIOA->CRL &= ~(0xFu << (1u * 4u));          /* PA1 아날로그 입력 */

    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    ADC1->SQR3  = 1u;                            /* 1차 변환 채널 = 1 (PA1) */
    ADC1->SMPR2 |= (7u << (3u * 1u));            /* 채널1 샘플시간 239.5 cycle */

    ADC1->CR2 |= ADC_CR2_ADON;
    short_delay();
    ADC1->CR2 |= ADC_CR2_RSTCAL; while (ADC1->CR2 & ADC_CR2_RSTCAL) { }
    ADC1->CR2 |= ADC_CR2_CAL;    while (ADC1->CR2 & ADC_CR2_CAL)    { }
}

static uint16_t adc1_read(void)
{
    ADC1->CR2 |= ADC_CR2_ADON;
    while (!(ADC1->SR & ADC_SR_EOC)) { }
    return (uint16_t)(ADC1->DR & 0x0FFFu);
}

int main(void)
{
    volatile uint16_t adc_val = 0;

    adc1_init();

    while (1) {
        adc_val = adc1_read();
        if (adc_val > 2048u) {
            GPIOC->BRR  = (1u << LED_PIN);   /* LED ON (액티브 로우) */
        } else {
            GPIOC->BSRR = (1u << LED_PIN);   /* LED OFF */
        }
    }
}
