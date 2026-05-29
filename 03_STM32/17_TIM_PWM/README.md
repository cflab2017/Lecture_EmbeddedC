# 17. 범용 타이머·PWM 출력

SysTick이 "정확한 시간"을 줬다면, 범용 타이머(TIM)는 그 위에 **채널·PWM·입력 캡처** 같은 기능을 얹습니다. 그중 가장 많이 쓰는 것이 **PWM(Pulse Width Modulation)** 입니다. 디지털 핀은 0 또는 1만 낼 수 있지만, 켜진 시간의 비율(듀티)을 빠르게 조절하면 LED 밝기나 모터 속도를 "아날로그처럼" 제어할 수 있습니다.

이번 편은 TIM2의 채널1(PA0)로 1kHz PWM을 만들고, 듀티를 바꿔 출력 비율을 조절합니다. 타이머의 세 핵심 레지스터 **PSC(분주)·ARR(주기)·CCR(듀티)** 의 관계를 이해하면, 원하는 주파수와 듀티를 자유롭게 계산할 수 있습니다.

## 학습 목표
- 범용 타이머의 시간축(PSC·ARR)과 카운터 클럭을 계산한다.
- PWM 모드1의 동작(CNT vs CCR 비교)을 이해한다.
- TIM2 채널1로 PWM을 출력하도록 레지스터를 설정한다.
- CCR로 듀티를, ARR로 주파수를 조절한다.
- APB1 타이머 클럭이 PCLK1의 2배가 되는 규칙을 안다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (TIM2 레지스터, Logic Analyzer PA0). 주파수/듀티 계산은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 타이머 시간축 — PSC와 ARR
타이머는 입력 클럭을 **PSC(prescaler)** 로 나눠 카운터 클럭을 만들고, 카운터가 **ARR(auto-reload)** 까지 올라가면 0으로 되돌아갑니다(업데이트 이벤트).

```
counter_clk = TIMxCLK / (PSC + 1)
pwm_freq    = counter_clk / (ARR + 1)
```

예: TIM2CLK=72MHz, PSC=71 → 카운터 1MHz. ARR=999 → 1MHz/1000 = **1kHz** PWM.

### 2) 타이머 클럭의 함정 — APB1 ×2
TIM2~4는 APB1 버스에 있습니다. **APB1 프리스케일러가 1이 아니면, 타이머 클럭은 PCLK1의 2배**가 됩니다(STM32 설계 규칙). 표준 72MHz 설정에서 PCLK1=36MHz지만 TIM2 클럭은 **72MHz**입니다. 그래서 위 예에서 72MHz를 썼습니다.

### 3) PWM 모드1 — CNT vs CCR
PWM 모드1에서 출력은 **`CNT < CCR`인 동안 High**, 그 뒤로 Low입니다. 따라서 한 주기(`ARR+1` 클럭) 중 High인 비율이 듀티입니다.

```
duty(%) = CCR * 100 / (ARR + 1)
```

CCR=250, ARR=999 → 250/1000 = **25%**. CCR을 키우면 더 밝게(더 오래 High).

### 4) PWM 설정 순서
| 단계 | 레지스터 | 내용 |
|------|----------|------|
| 클럭 | `RCC->APB1ENR`, `APB2ENR` | TIM2, GPIOA 켜기 |
| 핀 | `GPIOA->CRL` | PA0 = AF 푸시풀(0xB) |
| 시간축 | `PSC`, `ARR` | 카운터 클럭·주기 |
| 듀티 | `CCR1` | 비교값 |
| 모드 | `CCMR1` | OC1M=110(PWM1), OC1PE(프리로드) |
| 출력 | `CCER` | CC1E(채널 출력 인에이블) |
| 시작 | `CR1`, `EGR` | ARPE, UG(강제 로드), CEN(시작) |

### 5) 대체 기능(AF) 핀
타이머 출력은 GPIO의 **대체 기능(Alternate Function)** 입니다. PA0를 일반 출력이 아니라 AF 푸시풀(CNF=10, MODE=11 → 0xB)로 둬야 타이머 신호가 핀으로 나갑니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_pwm`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : TIM2 CH1 PWM (1kHz, 25%)
```c
#include "stm32f10x.h"

/*
 * 17강 예제 — TIM2 CH1(PA0) PWM 출력.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 타이머 클럭(여기선 72MHz)을 PSC 로 1MHz 로 나누고, ARR 로 주기를,
 * CCR1 로 듀티를 정한다. PWM 모드1: CNT < CCR1 동안 출력 High.
 *   - PSC=71  → 72MHz/(71+1) = 1MHz 카운터
 *   - ARR=999 → 1MHz/(999+1) = 1kHz PWM
 *   - CCR1=250 → 듀티 25%
 */

int main(void)
{
    /* 1) 클럭: GPIOA 는 APB2, TIM2 는 APB1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* 2) PA0 = TIM2_CH1 → 대체 기능 푸시풀 50MHz (CNF=10, MODE=11 → 0xB) */
    GPIOA->CRL &= ~(0xFu << (0u * 4u));
    GPIOA->CRL |=  (0xBu << (0u * 4u));

    /* 3) 타이머 시간축 */
    TIM2->PSC  = 71u;        /* 72MHz / 72 = 1MHz 카운터 */
    TIM2->ARR  = 999u;       /* 1MHz / 1000 = 1kHz PWM */
    TIM2->CCR1 = 250u;       /* 듀티 25% */

    /* 4) 채널1 PWM 모드1(110) + CCR 프리로드, 출력 인에이블 */
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;  /* OC1M = 110 */
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;                      /* CCR1 프리로드 */
    TIM2->CCER  |= TIM_CCER_CC1E;                        /* CH1 출력 켜기 */

    /* 5) ARR 프리로드 + 업데이트 강제 로드(UG) + 카운터 시작 */
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    while (1) {
        /* PWM 은 하드웨어가 자동 생성. CCR1 을 바꾸면 듀티가 즉시 바뀐다. */
    }
}
```

### 예제 2 — `pc_test.c` : 주파수·듀티 계산
```c
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
```

## 시뮬레이터로 확인하기
- `Peripherals → Timers → TIM2`에서 PSC=71, ARR=999, CCR1=250, CR1의 CEN=1을 확인.
- Logic Analyzer에 `PORTA.0`을 추가하면 1kHz 사각파에 25% 듀티(High 0.25ms, Low 0.75ms)가 보인다.
- 디버그 중 `TIM2->CCR1`을 500으로 바꾸면 듀티가 50%로 변한다.
- **예상 동작**: PA0에 1kHz PWM, 듀티는 CCR1에 비례.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
PSC=71 ARR=999 CCR=250 -> 1000 Hz, duty 25%
PSC=71 ARR=999 CCR=500 -> 1000 Hz, duty 50%
PSC=71 ARR=499 CCR=250 -> 2000 Hz, duty 50%
```

## 자주 하는 실수

### Q. 핀에서 PWM이 안 나와요.
A. 세 가지를 확인하세요. ① PA0를 **AF 푸시풀(0xB)** 로 설정했는지, ② `CCER`의 CC1E로 출력을 켰는지, ③ `CR1`의 CEN으로 카운터를 시작했는지.

### Q. 주파수가 계산의 절반/2배로 나와요.
A. 타이머 클럭을 잘못 잡았기 때문입니다. APB1 프리스케일러가 1이 아니면 TIM2 클럭은 PCLK1의 **2배**(72MHz)입니다. 36MHz로 계산하면 2배 어긋납니다.

### Q. 설정을 바꿨는데 첫 주기에 반영이 안 돼요.
A. `EGR`의 UG 비트로 업데이트 이벤트를 강제하면 PSC/ARR/CCR이 즉시 로드됩니다. 이를 빠뜨리면 다음 업데이트까지 옛 값으로 돕니다.

### Q. 듀티 100%를 주고 싶어요.
A. `CCR ≥ ARR+1`이면 `CNT < CCR`이 항상 참이라 출력이 계속 High(100%)입니다. ARR=999일 때 CCR=1000이면 100%입니다.

## 정리
- 타이머: PSC로 카운터 클럭, ARR로 주기, CCR로 듀티를 정한다.
- PWM 모드1은 `CNT < CCR` 동안 High → 듀티 = CCR/(ARR+1).
- 타이머 출력은 AF 핀(PA0=0xB)으로 내보낸다.
- 설정 순서: 클럭 → 핀 → 시간축 → 모드/출력 → UG → CEN.
- APB1 타이머 클럭은 PCLK1의 2배(72MHz)임에 주의.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[18. NVIC 인터럽트와 EXTI](../18_NVIC_EXTI/README.md) — 폴링을 버리고 인터럽트로 넘어갑니다. 외부 핀 변화(EXTI)를 NVIC로 받아 즉시 반응하는 법을 배웁니다.
