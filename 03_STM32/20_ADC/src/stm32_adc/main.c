#include "stm32f10x.h"

/*
 * 20강 예제 — ADC1 채널0(PA0) 단일 변환.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 12비트 ADC(0~4095)로 PA0 의 전압을 읽어 밀리볼트로 환산한다.
 *   mV = adc * 3300 / 4095   (기준 전압 3.3V 가정)
 * F1 ADC 는 켠 뒤 보정(RSTCAL→CAL)을 한 번 해야 정확하다.
 */

static void short_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 10000u; i++) { }   /* tSTAB 안정화용 짧은 대기 */
}

static void adc1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;
    RCC->CFGR    |= RCC_CFGR_ADCPRE_DIV6;       /* ADC 클럭 72/6=12MHz (≤14MHz) */

    /* PA0 = 아날로그 입력 (CNF=00, MODE=00 → 0x0) */
    GPIOA->CRL &= ~(0xFu << (0u * 4u));

    ADC1->SQR3  = 0u;                            /* 1차 변환 채널 = 0 (PA0) */
    ADC1->SMPR2 |= (7u << (3u * 0u));            /* 채널0 샘플시간 239.5 cycle */

    ADC1->CR2 |= ADC_CR2_ADON;                   /* ADC 전원 ON */
    short_delay();

    ADC1->CR2 |= ADC_CR2_RSTCAL;                 /* 보정 레지스터 리셋 */
    while (ADC1->CR2 & ADC_CR2_RSTCAL) { }
    ADC1->CR2 |= ADC_CR2_CAL;                    /* 보정 시작 */
    while (ADC1->CR2 & ADC_CR2_CAL) { }          /* 보정 완료 대기 */
}

static uint16_t adc1_read(void)
{
    ADC1->CR2 |= ADC_CR2_ADON;                   /* ADON 재설정 = 변환 시작 */
    while (!(ADC1->SR & ADC_SR_EOC)) { }         /* 변환 완료(EOC) 대기 */
    return (uint16_t)(ADC1->DR & 0x0FFFu);       /* 12비트 결과 */
}

int main(void)
{
    volatile uint16_t adc_val = 0;
    volatile uint32_t mv = 0;

    adc1_init();

    while (1) {
        adc_val = adc1_read();
        mv = (uint32_t)adc_val * 3300u / 4095u;  /* 밀리볼트 환산 */
    }
}
