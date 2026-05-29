# 13. Cortex-M·CMSIS·클럭 트리

여기서 8051을 졸업하고 **32비트 Arm Cortex-M**(STM32F103)으로 무대를 옮깁니다. 8비트 8051과 비교하면 레지스터 폭, 주소 공간, 주변장치 수가 한 단계 넓어지고, 무엇보다 **CMSIS**라는 표준 덕분에 어느 Cortex-M이든 비슷한 방식으로 레지스터에 접근합니다. 이 편은 코드를 많이 쓰기보다, 앞으로 7편(14~20)을 떠받칠 세 가지 토대 — **Cortex-M 구조, CMSIS 헤더, 클럭 트리** — 를 잡는 데 집중합니다.

이 트랙은 STM32 HAL/SPL 라이브러리를 쓰지 않고 **레지스터를 직접 제어**합니다. 추상화 함수 뒤에 숨은 동작을 직접 보는 것이 학습 목적이기 때문입니다. 대신 CMSIS가 제공하는 표준 심볼(`RCC->APB2ENR`, `GPIOC->ODR`, `SystemCoreClock` 등)을 사용해 가독성과 이식성을 지킵니다.

> **시뮬레이터 메모**: μVision의 STM32F103 시뮬레이터는 코어와 RCC·GPIO 같은 기본 레지스터를 모델링합니다. 이 편 예제는 RCC 레지스터 비트 변화와 변수 증가를 레지스터/Watch 창에서 관찰하고, 클럭 트리 계산은 `pc_test.c`로 PC에서 검증합니다.

## 학습 목표
- Cortex-M(STM32)과 8051의 구조 차이(폭·주소·버스)를 설명한다.
- CMSIS가 무엇이고 `stm32f10x.h`가 어떤 심볼을 제공하는지 안다.
- "주변장치를 쓰기 전에 클럭부터 켠다"는 RCC 규칙을 적용한다.
- HSE/HSI·PLL·AHB/APB 프리스케일러로 이어지는 클럭 트리를 따라간다.
- 설정값으로부터 SYSCLK/HCLK/PCLK1/PCLK2를 계산한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (Peripherals/Registers, Watch 창). 클럭 계산은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) Cortex-M과 8051 — 무엇이 달라지나
| 항목 | 8051 (AT89C52) | Cortex-M3 (STM32F103) |
|------|----------------|------------------------|
| 폭 | 8비트 | 32비트 |
| 클럭 | 보통 ~12MHz | 최대 72MHz |
| 레지스터 접근 | SFR (`P1`, `TMOD` …) | 메모리 맵드 구조체 (`GPIOC->ODR` …) |
| 주변장치 클럭 | 항상 켜짐 | **RCC로 개별 인에이블 필요** |
| 표준 헤더 | `reg52.h` | CMSIS `stm32f10x.h` |

가장 중요한 새 습관은 **클럭 게이팅**입니다. 8051은 포트가 늘 살아 있지만, STM32는 전력을 아끼려고 각 주변장치 클럭이 기본적으로 꺼져 있어, 쓰기 전에 RCC로 켜 줘야 합니다.

### 2) CMSIS와 `stm32f10x.h`
**CMSIS**(Cortex Microcontroller Software Interface Standard)는 Arm이 정한 표준 계층입니다. 우리가 쓰는 부분은 두 가지입니다.

- **CMSIS-CORE**: 코어 주변장치(NVIC, SysTick)와 인트린식(`__NOP`, `NVIC_EnableIRQ` 등). `core_cm3.h`.
- **디바이스 헤더 `stm32f10x.h`**: ST가 제공하며, 모든 주변장치를 C 구조체로 정의합니다.

```c
/* stm32f10x.h 가 제공하는 형태(개념) */
RCC->APB2ENR  |= RCC_APB2ENR_IOPCEN;  /* 비트 이름이 매크로로 정의됨 */
GPIOC->ODR    ^= (1u << 13);          /* 포트 = 주소가 박힌 구조체 포인터 */
```

`RCC`, `GPIOC`는 정해진 주소를 가리키는 구조체 포인터고, `RCC_APB2ENR_IOPCEN` 같은 비트 마스크도 헤더에 정의돼 있어 매직 넘버를 피할 수 있습니다.

### 3) RCC — 클럭을 켜는 관문
RCC(Reset and Clock Control)는 클럭 소스와 주변장치 클럭 게이트를 모두 관리합니다. 주변장치마다 어느 버스에 매달렸는지에 따라 인에이블 레지스터가 다릅니다.

| 버스 | 인에이블 레지스터 | 대표 주변장치 |
|------|-------------------|---------------|
| AHB | `RCC->AHBENR` | DMA, CRC |
| APB2 (고속) | `RCC->APB2ENR` | GPIOA~E, ADC1/2, USART1, TIM1, AFIO |
| APB1 (저속) | `RCC->APB1ENR` | TIM2~4, USART2/3, I2C, SPI2 |

```c
RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;  /* GPIOA, GPIOC 켜기 */
```

### 4) 클럭 트리
STM32의 클럭은 소스에서 출발해 분주/체배를 거쳐 각 버스로 흘러갑니다.

```
HSI(내부 8MHz) ─┐
                ├─► (PLL ×배수) ─► SYSCLK ─► AHB(/HPRE) ─► HCLK ─┬─► APB1(/PPRE1) ─► PCLK1
HSE(외부 8MHz) ─┘                                                  └─► APB2(/PPRE2) ─► PCLK2
```

- **HSI**: 내부 RC 발진기(~8MHz), 부정확하지만 외부 부품 불필요.
- **HSE**: 외부 크리스털(보통 8MHz), 정확. PLL과 함께 고속 클럭을 만든다.
- **PLL**: 입력을 정수배로 체배. F103은 8MHz×9 = **72MHz**가 표준.
- **프리스케일러**: SYSCLK를 버스별로 나눠 한계 내로 맞춘다.

### 5) 버스 클럭 한계 — APB1이 /2인 이유
F103의 한계는 SYSCLK/HCLK ≤ 72MHz, **PCLK1(APB1) ≤ 36MHz**, PCLK2(APB2) ≤ 72MHz입니다. 그래서 표준 72MHz 설정에서 APB1은 /2(=36MHz), APB2는 /1(=72MHz)로 둡니다. 이 계산을 `pc_test.c`에서 직접 해 봅니다.

> 참고: APB 프리스케일러가 1이 아니면 그 버스의 타이머 클럭은 PCLK의 2배가 됩니다(타이머 편 17편에서 다룸). 이 편 계산에서는 단순화를 위해 버스 클럭까지만 다룹니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 폴더·이름 `stm32_intro`.
2. Device: **STMicroelectronics → STM32F1 Series → STM32F103 → STM32F103C8**.
3. `Manage Run-Time Environment` 창에서 **CMSIS → CORE** 체크, **Device → Startup** 체크. (이러면 `stm32f10x.h`와 startup, `system_stm32f10x.c`가 자동 포함된다.)
4. `main.c`를 프로젝트에 추가하고 아래 코드를 복붙.
5. `Options for Target → C/C++ → Define`에 디바이스 매크로 `STM32F10X_MD` 추가. (C8은 64KB 플래시 = Medium Density)
6. `Options for Target → Debug → Use Simulator` 선택. Dialog DLL 파라미터에 `-pSTM32F103C8`.
7. Rebuild(F7) → Start Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : 주변장치 클럭 켜기
GPIOA·GPIOC의 클럭을 켜는 CMSIS 표준 관용구입니다. 동작 자체보다, RCC 레지스터 비트가 바뀌는 것을 눈으로 확인하는 게 목표입니다.

```c
#include "stm32f10x.h"

/*
 * 13강 예제 — CMSIS 레지스터 접근의 기본형.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM (Arm Compiler 6).
 *
 * 이 편은 '구조 이해'가 목표다. 실제 GPIO 출력으로 LED 를 켜는 일은 14편에서
 * 한다. 여기서는 CMSIS 헤더로 주변장치 클럭을 켜는 표준 관용구를 보여 주고,
 * 그 결과(RCC 레지스터 비트)가 시뮬레이터 레지스터 창에서 바뀌는 것을 관찰한다.
 *
 * STM32 의 철칙: "주변장치를 쓰기 전에 먼저 그 주변장치의 클럭을 켠다."
 * 클럭이 꺼진 주변장치는 레지스터에 값을 써도 반응하지 않는다.
 */

int main(void)
{
    volatile uint32_t heartbeat = 0;   /* 살아 있음을 보여 주는 카운터 */

    /* GPIOA, GPIOC 는 APB2 버스에 매달려 있다. 두 포트의 클럭을 켠다.
     * 켜기 전에는 GPIOA->/GPIOC-> 레지스터가 동작하지 않는다. */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    /* SystemCoreClock 은 CMSIS 가 관리하는 현재 코어 클럭(Hz) 전역 변수다.
     * Watch 창에 추가하면 system_stm32f10x.c 설정값(보통 72000000)이 보인다. */

    while (1) {
        heartbeat++;       /* Watch 창에서 값이 증가하는 것을 관찰 */
    }
}
```

### 예제 2 — `pc_test.c` : 클럭 트리 계산
설정값(HSE·PLL배수·프리스케일러)으로부터 각 버스 주파수를 계산합니다. 하드웨어 없이 클럭 트리를 손으로 따라가는 연습입니다.

```c
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
```

## 시뮬레이터로 확인하기
- 디버그 시작 후 `Peripherals → System Viewer → RCC`(또는 Registers 창)에서 `APB2ENR`를 본다. `main` 한 줄을 지나면 **IOPAEN(비트2)·IOPCEN(비트4)** 가 1이 되어 값이 `0x00000014`가 된다.
- `heartbeat`를 Watch 창에 추가하고 실행하면 값이 계속 증가한다.
- `SystemCoreClock`을 Watch에 추가하면 설정된 코어 클럭(보통 `72000000`)이 보인다.
- **예상 동작**: 클럭 인에이블 비트가 세팅되고, 무한 루프에서 카운터가 증가.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
[72MHz standard]
  SYSCLK = 72000000 Hz
  HCLK   = 72000000 Hz
  PCLK1  = 36000000 Hz
  PCLK2  = 72000000 Hz
[32MHz example]
  SYSCLK = 32000000 Hz
  HCLK   = 32000000 Hz
  PCLK1  = 32000000 Hz
  PCLK2  = 32000000 Hz
```

## 자주 하는 실수

### Q. GPIO 레지스터에 값을 썼는데 아무 일도 안 일어나요.
A. 십중팔구 **클럭을 안 켰습니다**. STM32 주변장치는 기본적으로 클럭이 꺼져 있어, `RCC->APB2ENR |= RCC_APB2ENR_IOPxEN;`을 먼저 해야 레지스터가 살아납니다. 8051과 가장 다른 점입니다.

### Q. `stm32f10x.h`를 못 찾거나 `RCC`가 정의되지 않았다고 나와요.
A. Manage Run-Time Environment에서 **CMSIS:CORE와 Device:Startup**을 체크했는지, 그리고 C/C++ Define에 **`STM32F10X_MD`** 같은 디바이스 매크로를 넣었는지 확인하세요. 밀도 매크로가 없으면 헤더가 어떤 칩인지 몰라 컴파일이 막힙니다.

### Q. 어떤 주변장치가 APB1인지 APB2인지 매번 헷갈려요.
A. 대략 "빠른(고속) 것은 APB2, 느린 것은 APB1"로 기억하되, 데이터시트의 클럭 트리 그림이 정답입니다. GPIO·ADC·USART1·TIM1은 APB2, TIM2~4·USART2/3·I2C·SPI2는 APB1입니다.

### Q. HSI가 8MHz인데 어떻게 72MHz가 되나요?
A. PLL이 체배합니다. HSE(또는 HSI/2)를 입력으로 받아 ×9 하면 72MHz입니다. system_stm32f10x.c의 `SystemInit()`이 startup 단계에서 이 설정을 적용하고, `SystemCoreClock`에 결과를 기록합니다.

## 정리
- STM32는 32비트 Cortex-M으로, 주변장치마다 RCC로 **클럭을 켜야** 동작한다.
- CMSIS `stm32f10x.h`가 레지스터를 구조체·비트 매크로로 제공해 직접 제어를 돕는다.
- 클럭 트리: 소스(HSI/HSE) → PLL → SYSCLK → AHB/APB 프리스케일러 → 각 버스.
- F103 한계로 APB1은 /2(36MHz), APB2는 /1(72MHz)이 표준이다.
- HAL/SPL 없이 레지스터를 직접 다루며, CMSIS 표준 심볼로 가독성을 지킨다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[14. RCC·GPIO 레지스터로 LED](../14_RCC_GPIO_LED/README.md) — 이론을 실전으로. RCC로 GPIO 클럭을 켜고, 모드 레지스터로 핀을 출력으로 설정해 PC13의 온보드 LED를 직접 깜빡입니다.
