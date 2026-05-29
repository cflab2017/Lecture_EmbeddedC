# 15. GPIO 입력·풀업/풀다운

14편이 핀으로 신호를 내보냈다면, 이번엔 핀으로 신호를 **읽습니다**. 버튼·스위치 같은 입력은 임베디드의 기본인데, 여기엔 초보자가 꼭 걸리는 함정이 하나 있습니다. 바로 **플로팅(floating) 입력** — 아무것도 연결되지 않은 입력 핀은 0도 1도 아닌 떠 있는 상태라 노이즈에 따라 값이 흔들립니다. 이를 막는 것이 **풀업/풀다운 저항**이고, STM32는 이를 칩 내부에 내장하고 있어 레지스터로 켤 수 있습니다.

이번 편은 PA0를 입력으로 설정하고 내부 풀업을 켠 뒤, 버튼 상태를 `IDR`로 읽어 PC13 LED를 제어합니다. 입력 모드의 4비트 설정, 풀업/풀다운을 ODR로 고르는 방식, 그리고 능동 논리(active low/high)를 익힙니다.

## 학습 목표
- GPIO를 입력 모드로 설정하고 `IDR`로 핀 상태를 읽는다.
- 플로팅 입력의 위험과 풀업/풀다운의 필요성을 설명한다.
- F1에서 내부 풀업/풀다운을 ODR 비트로 선택한다.
- 액티브 로우/하이 버튼 배선에 따른 판정 논리를 구현한다.
- 버튼 입력으로 LED를 켜고 끈다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (GPIOA IDR 강제 입력, GPIOC 관찰). 판정 로직은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 입력 모드 4비트
입력은 MODE=`00`이고, CNF가 입력의 종류를 정합니다.

| CNF | 입력 종류 | 4비트 코드 |
|-----|-----------|------------|
| 00 | 아날로그 | `0x0` |
| 01 | 플로팅 입력(리셋 기본) | `0x4` |
| 10 | 풀업/풀다운 입력 | `0x8` |

버튼 입력은 보통 **풀업/풀다운 입력(0x8)** 을 씁니다.

### 2) 플로팅이 위험한 이유
입력 핀에 아무 연결이 없으면, 핀 전압이 주변 노이즈에 따라 0과 1 사이를 떠돕니다. 버튼을 누르지 않았는데 눌린 것처럼 읽히는 채터링의 원인이 되죠. **풀업**(평소 1로 고정) 또는 **풀다운**(평소 0으로 고정)으로 기본값을 정해 주면 해결됩니다.

### 3) 풀업/풀다운 선택은 ODR로
F1에서 CNF=`10`(풀업/풀다운 입력)일 때, **어느 쪽인지는 ODR 비트가 정합니다**.

| ODR 비트 | 효과 |
|----------|------|
| 1 | 내부 풀업 (평소 IDR=1) |
| 0 | 내부 풀다운 (평소 IDR=0) |

```c
GPIOA->CRL |= (0x8u << 0);   /* PA0 = 풀업/풀다운 입력 */
GPIOA->ODR |= (1u << 0);     /* ODR=1 → 풀업 */
```

### 4) 읽기 — IDR
`IDR`(Input Data Register)의 해당 비트가 핀 상태입니다. 비트 하나만 보려면 마스크합니다.

```c
if ((GPIOA->IDR & (1u << 0)) == 0u) { /* PA0 가 0 (눌림) */ }
```

### 5) 능동 논리 — 풀업 + GND 버튼 = 액티브 로우
버튼을 PA0와 GND 사이에 달고 풀업을 켜면:
- 안 누름 → 풀업이 1로 유지 → IDR=1
- 누름 → GND 연결 → IDR=0

즉 **눌림 = 0**(액티브 로우)입니다. 풀다운 + VCC 버튼이면 반대로 눌림=1(액티브 하이)입니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_gpio_in`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : 버튼으로 LED 제어
```c
#include "stm32f10x.h"

/*
 * 15강 예제 — 버튼 입력으로 LED 제어.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * PA0 를 '입력 + 내부 풀업'으로 두고, 버튼은 PA0 와 GND 사이에 단다.
 *   - 안 누름: 풀업이 1 로 끌어올림 → IDR 비트 = 1
 *   - 누름:    GND 로 연결 → IDR 비트 = 0   (액티브 로우 버튼)
 * 버튼이 눌리면 PC13 LED 를 켠다(LED 도 액티브 로우라 핀=0 이 ON).
 */

#define BTN_PIN   0u     /* PA0 */
#define LED_PIN   13u    /* PC13 */

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    /* PA0: 입력 + 풀업/풀다운. CNF=10, MODE=00 → 4비트 0x8.
     * 풀업이냐 풀다운이냐는 ODR 비트로 고른다(ODR=1 → 풀업). */
    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));
    GPIOA->ODR |=  (1u << BTN_PIN);                    /* 풀업 선택 */

    /* PC13: 푸시풀 출력 2MHz = 0x2 (CRH) */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    while (1) {
        if ((GPIOA->IDR & (1u << BTN_PIN)) == 0u) {
            GPIOC->BRR  = (1u << LED_PIN);   /* 눌림: LED ON (=0) */
        } else {
            GPIOC->BSRR = (1u << LED_PIN);   /* 안 눌림: LED OFF (=1) */
        }
    }
}
```

### 예제 2 — `pc_test.c` : 입력 판정 로직 검산
```c
#include <stdio.h>
#include <stdint.h>

/*
 * 15강 PC 검산 — 풀업 입력에서 버튼 눌림 판정과 LED 결정 로직.
 * 풀업 + 버튼이 GND 로 연결되면, 안 누르면 1(풀업), 누르면 0(액티브 로우).
 * 하드웨어 없이 여러 IDR 값에 대한 판정을 확인한다.
 */

static int is_pressed(uint32_t idr, unsigned int pin)
{
    return ((idr & (1u << pin)) == 0u) ? 1 : 0;   /* 비트가 0 이면 눌림 */
}

int main(void)
{
    uint32_t samples[4] = { 0x0001u, 0x0000u, 0x0021u, 0x0020u };
    unsigned int pin = 0u;
    int i;

    for (i = 0; i < 4; i++) {
        int pressed = is_pressed(samples[i], pin);
        printf("IDR=0x%04lX -> %s -> LED %s\n",
               (unsigned long)samples[i],
               pressed ? "PRESSED" : "released",
               pressed ? "ON" : "OFF");
    }
    return 0;
}
```

## 시뮬레이터로 확인하기
- 시뮬레이터에서는 버튼이 없으니, `Peripherals → GPIOA`의 IDR을 직접 바꾸거나 디버그 명령으로 핀을 강제합니다. 예: Command 창에 `PORTA &= ~0x0001`(PA0=0, 눌림) / `PORTA |= 0x0001`(PA0=1, 안 눌림).
- PA0=0으로 만들면 PC13이 0(LED ON), PA0=1이면 PC13이 1(LED OFF)이 되는지 GPIOC ODR로 확인.
- **예상 동작**: PA0 입력에 따라 PC13이 즉시 반대로 따라간다.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
IDR=0x0001 -> released -> LED OFF
IDR=0x0000 -> PRESSED -> LED ON
IDR=0x0021 -> released -> LED OFF
IDR=0x0020 -> PRESSED -> LED ON
```

## 자주 하는 실수

### Q. 버튼을 안 눌렀는데 값이 마구 흔들려요.
A. 풀업/풀다운을 안 켜서 플로팅 상태입니다. CNF=10(0x8)으로 두고 ODR로 풀업(1) 또는 풀다운(0)을 선택하세요.

### Q. CNF=10으로 했는데도 풀업이 안 걸려요.
A. F1은 ODR 비트로 풀업/풀다운을 고릅니다. 입력 모드에서 ODR=1이면 풀업, 0이면 풀다운입니다. ODR 설정을 빠뜨리지 마세요.

### Q. `IDR`에 쓰면 핀이 바뀌나요?
A. IDR은 **읽기 전용**입니다. 출력은 ODR/BSRR/BRR로 합니다. IDR은 핀의 현재 입력 상태를 읽기만 합니다.

### Q. 버튼이 가끔 두 번 눌린 것처럼 동작해요.
A. 기계식 버튼은 접점이 튀는 채터링(bounce)이 있습니다. 이 편은 단순 읽기까지만 다루고, 디바운싱은 21편에서 상태 머신으로 제대로 처리합니다.

## 정리
- 입력은 MODE=00, CNF로 종류 결정(0x8 = 풀업/풀다운 입력).
- 플로팅을 피하려 풀업/풀다운을 켜고, F1은 그 선택을 ODR로 한다.
- 핀 상태는 읽기 전용 `IDR`로 읽는다.
- 풀업 + GND 버튼 = 눌림이 0(액티브 로우).
- 채터링 처리는 디바운싱(21편)에서.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[16. SysTick 타이머와 정확한 지연](../16_SysTick/README.md) — 부정확한 `delay()` 루프를 버리고, Cortex-M 코어 내장 SysTick으로 정확한 밀리초 지연을 만듭니다.
