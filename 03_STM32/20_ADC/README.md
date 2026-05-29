# 20. ADC 아날로그 입력

지금까지 다룬 핀은 모두 0 또는 1, 즉 **디지털**이었습니다. 하지만 세상의 신호 대부분은 온도·밝기·소리처럼 연속적인 **아날로그**입니다. **ADC**(Analog-to-Digital Converter)는 아날로그 전압을 숫자로 바꿔, MCU가 그 값을 다룰 수 있게 합니다. 가변저항, 조도센서, 온도센서 같은 입력이 모두 ADC로 들어옵니다.

STM32F103의 ADC는 **12비트**라 전압을 0~4095의 정수로 표현합니다. 이번 편은 ADC1으로 PA0의 전압을 읽고, 그 카운트를 밀리볼트로 환산합니다. STM32 주변장치 시리즈(13~20편)의 마지막 편으로, 앞서 배운 클럭·핀·폴링이 모두 한 번에 쓰입니다.

> **시뮬레이터 메모**: μVision의 STM32F103 ADC 모델은 입력 전압 주입이 제한적일 수 있습니다. 이 편은 변환 시퀀스(보정·EOC·DR)와 전압 환산 로직을 레지스터/Watch로 확인하고, **카운트→전압 변환 공식 자체는 `pc_test.c`로 PC에서 검증**합니다.

## 학습 목표
- ADC의 역할과 12비트 분해능(0~4095), 기준 전압 개념을 이해한다.
- ADC 클럭 제약(≤14MHz)과 프리스케일러를 설정한다.
- 채널·샘플 시간·변환 시퀀스를 설정한다.
- F1 ADC의 보정(RSTCAL/CAL)과 변환 완료(EOC) 폴링을 구현한다.
- ADC 카운트를 밀리볼트로 환산한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (ADC1 레지스터, Watch). 환산 공식은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 분해능과 기준 전압
12비트 ADC는 입력 전압 범위(0 ~ Vref, 보통 3.3V)를 `2¹² = 4096` 단계로 나눕니다. 한 단계(LSB)는 약 `3300mV / 4095 ≈ 0.806mV`입니다.

```
mV = adc * 3300 / 4095
```

| ADC 카운트 | 전압 |
|-----------|------|
| 0 | 0 mV |
| 2048 | 약 1650 mV |
| 4095 | 3300 mV |

### 2) ADC 클럭 제약
ADC 클럭(ADCCLK)은 **최대 14MHz**입니다. PCLK2=72MHz를 그대로 쓰면 너무 빠르므로 `RCC->CFGR`의 ADCPRE로 나눕니다. /6이면 12MHz로 한계 안에 듭니다.

```c
RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;   /* 72/6 = 12MHz */
```

### 3) 채널과 샘플 시간
- **채널**: PA0=채널0, PA1=채널1 … 변환할 채널을 시퀀스 레지스터 `SQR3`의 첫 자리(SQ1)에 넣습니다.
- **샘플 시간**: `SMPR`로 정합니다. 임피던스가 높은 소스는 길게(예: 239.5 cycle) 잡아야 정확합니다.

```c
ADC1->SQR3  = 0u;                 /* 1차 변환 채널 0 (PA0) */
ADC1->SMPR2 |= (7u << 0);         /* 채널0 샘플시간 최대 */
```

### 4) 보정과 변환(F1 특유)
F1 ADC는 켠 뒤 **보정**을 한 번 해야 정확합니다.

```c
ADC1->CR2 |= ADC_CR2_ADON;             /* 전원 ON */
/* tSTAB 대기 */
ADC1->CR2 |= ADC_CR2_RSTCAL; while (ADC1->CR2 & ADC_CR2_RSTCAL) { }
ADC1->CR2 |= ADC_CR2_CAL;    while (ADC1->CR2 & ADC_CR2_CAL)    { }
```

변환은 ADON을 **다시 세팅**해 시작하고, `SR`의 **EOC**(변환 완료)를 기다린 뒤 `DR`을 읽습니다.

### 5) 결과 읽기
`DR`은 12비트 결과를 담습니다(우측 정렬 기본). 읽으면 EOC가 클리어됩니다.

```c
ADC1->CR2 |= ADC_CR2_ADON;
while (!(ADC1->SR & ADC_SR_EOC)) { }
uint16_t v = ADC1->DR & 0x0FFF;
```

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_adc`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : PA0 전압 읽기
```c
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
```

### 예제 2 — `pc_test.c` : 카운트→전압 환산
```c
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
```

## 시뮬레이터로 확인하기
- `adc_val`과 `mv`를 Watch에 추가하고 실행한다.
- `Peripherals → A/D Converter → ADC1`에서 입력 전압을 설정할 수 있으면(시뮬 버전에 따라 다름) 값을 바꾸며 `DR`과 `mv` 변화를 관찰한다.
- ADC1의 CR2에서 ADON=1, 보정 후 CAL=0, 변환 후 SR의 EOC=1을 확인.
- **예상 동작**: 입력 전압에 비례해 `adc_val`(0~4095), `mv`(0~3300)가 갱신된다.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
ADC=0 -> 0 mV
ADC=1024 -> 825 mV
ADC=2048 -> 1650 mV
ADC=3072 -> 2475 mV
ADC=4095 -> 3300 mV
```

## 자주 하는 실수

### Q. 변환이 끝나지 않고 EOC 대기에서 멈춰요.
A. ADC 전원(ADON)을 켰는지, 변환을 시작(ADON 재설정 또는 SWSTART)했는지 확인하세요. 또 ADC 클럭이 활성(`RCC_APB2ENR_ADC1EN`)인지도요.

### Q. 값이 들쭉날쭉하고 부정확해요.
A. ① F1은 보정(RSTCAL/CAL)을 빠뜨리면 부정확합니다. ② 샘플 시간이 짧으면 고임피던스 소스에서 오차가 큽니다. ③ ADC 클럭이 14MHz를 넘으면 안 됩니다(ADCPRE 확인).

### Q. mV 계산이 0이 나와요.
A. `adc/4095*3300` 순서면 정수에서 0이 됩니다. 반드시 **곱한 뒤 나누기**: `adc * 3300 / 4095`. 그리고 32비트로 곱해 오버플로를 피하세요(`(uint32_t)adc * 3300`).

### Q. PA0을 입력 풀업으로 뒀어요.
A. ADC 입력은 **아날로그 입력(0x0)** 이어야 합니다. 디지털 입력 버퍼/풀업이 켜져 있으면 측정이 왜곡됩니다.

## 정리
- ADC는 아날로그 전압을 12비트 정수(0~4095)로 바꾼다.
- mV = adc × 3300 / 4095 (곱한 뒤 나누고, 32비트로 계산).
- ADC 클럭은 ≤14MHz — ADCPRE로 나눈다.
- F1은 보정(RSTCAL/CAL) 후 변환, EOC 폴링 뒤 DR을 읽는다.
- ADC 핀은 아날로그 입력(0x0)으로 설정한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[21. 디바운싱·유한 상태 머신 패턴](../../04_응용/21_디바운싱_상태머신/README.md) — 주변장치를 졸업하고 '응용·패턴' 단원으로 갑니다. 버튼 채터링을 잡는 디바운싱과, 펌웨어의 뼈대가 되는 상태 머신을 순수 로직으로 익힙니다.
