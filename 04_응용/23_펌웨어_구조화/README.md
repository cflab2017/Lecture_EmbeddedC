# 23. HAL 추상화·모듈 분리·이식성

지금까지 LED를 켜는 코드는 `GPIOC->BRR = ...`처럼 레지스터를 직접 건드렸습니다. 그런데 만약 칩을 STM32에서 8051로 바꾼다면? 또는 같은 동작을 PC에서 테스트하고 싶다면? 레지스터 코드가 응용 로직 곳곳에 박혀 있으면, 모든 곳을 다시 고쳐야 합니다. **HAL(Hardware Abstraction Layer)** 은 이 문제를 푸는 설계입니다. "무엇을 한다"(LED를 켠다)는 응용과, "어떻게 한다"(특정 레지스터를 쓴다)는 구현을 갈라놓는 것이죠.

이번 편은 같은 응용 로직(`app.c`)을 **두 가지 HAL 구현**으로 돌려 봅니다 — PC용(`hal_pc.c`, printf)과 STM32용(`hal_stm32.c`, 레지스터). 응용 코드는 한 줄도 바뀌지 않습니다. 이것이 모듈 분리가 주는 이식성과 테스트 용이성입니다.

## 학습 목표
- 펌웨어를 응용/HAL/하드웨어 계층으로 나누는 이유를 설명한다.
- 인터페이스(헤더)와 구현(소스)을 분리한다.
- 같은 응용을 여러 HAL 구현으로 이식한다.
- 의존 방향(응용→HAL, 역방향 금지)을 지킨다.
- PC용 가짜 HAL로 응용 로직을 단위 테스트한다.

## 대상 환경
- 컴파일러: **PC + gcc**(응용+PC HAL) / Keil MDK(응용+STM32 HAL).
- 디바이스: 없음(응용·HAL 인터페이스는 비의존). STM32 HAL은 STM32F103C8.
- 검증: PC `gcc`(`pc_test.c` + `app.c` + `hal_pc.c`).

## 핵심 개념

### 1) 계층화(layering)
펌웨어를 책임에 따라 층으로 나눕니다. 위층은 아래층만 호출하고, 아래층은 위층을 모릅니다.

```
┌────────────────────┐
│ 응용 (app.c)        │  "패턴대로 LED 제어" — 하드웨어 모름
├────────────────────┤
│ HAL 인터페이스(hal.h)│  "led_set(on)" 같은 계약
├────────────────────┤
│ HAL 구현            │  hal_pc.c (printf)  /  hal_stm32.c (레지스터)
├────────────────────┤
│ 하드웨어            │  GPIO, ...
└────────────────────┘
```

### 2) 인터페이스와 구현의 분리
헤더(`hal.h`)는 **계약**입니다 — 함수 이름·인자·의미만 정합니다. 소스(`hal_pc.c`, `hal_stm32.c`)는 그 계약을 각자의 방식으로 **구현**합니다. 응용은 헤더만 `#include`하고 어떤 구현이 링크될지는 신경 쓰지 않습니다.

### 3) 이식성 — 구현만 교체
```
PC 테스트:   pc_test.c + app.c + hal_pc.c     → printf 로 동작 확인
실제 보드:   main.c    + app.c + hal_stm32.c  → PC13 LED 동작
```
`app.c`는 양쪽에서 **동일**합니다. 칩을 바꾸려면 HAL 구현만 새로 쓰면 됩니다.

### 4) 의존 방향 규칙
- 응용 → HAL : 허용 (위가 아래를 호출)
- HAL → 응용 : **금지** (아래가 위를 알면 분리가 깨진다)

이 방향을 지켜야 HAL을 다른 프로젝트에 떼어다 재사용할 수 있습니다.

### 5) 테스트 용이성
하드웨어가 없어도, PC용 가짜 HAL을 끼우면 응용 로직을 `gcc`로 즉시 검증할 수 있습니다. 로직 버그를 보드 없이 빠르게 잡는 것이 큰 이점입니다(이 트랙의 `pc_test.c`가 바로 이 방식).

## 예제로 보기

### 예제 1 — `hal.h` : HAL 계약
```c
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

/*
 * 하드웨어 추상화 계층(HAL) 인터페이스.
 *
 * 응용 코드는 이 함수들만 호출한다. "어떻게" 켜고 끄는지(레지스터인지
 * printf 인지)는 모른다. 구현 파일을 갈아끼우면(hal_pc.c ↔ hal_stm32.c)
 * 같은 응용 로직이 PC 에서도, 실제 보드에서도 동작한다.
 */
void hal_led_init(void);
void hal_led_set(uint8_t on);    /* 1 = 켜기, 0 = 끄기 */

#endif /* HAL_H */
```

### 예제 2 — `app.c` : 하드웨어 비의존 응용
```c
#include "app.h"
#include "hal.h"

/*
 * 응용 계층 — HAL 만 호출한다. 여기에는 레지스터 코드가 한 줄도 없다.
 * 그래서 이 파일은 칩이 바뀌어도 수정할 필요가 없다(이식성의 핵심).
 */
void app_blink_pattern(const uint8_t *pattern, uint16_t len)
{
    uint16_t i;

    hal_led_init();
    for (i = 0; i < len; i++) {
        hal_led_set(pattern[i] ? 1u : 0u);
    }
}
```

### 예제 3 — 두 가지 HAL 구현
PC용(검증):
```c
#include "hal.h"
#include <stdio.h>

void hal_led_init(void)        { printf("[HAL-PC] led init\n"); }
void hal_led_set(uint8_t on)   { printf("[HAL-PC] LED %s\n", on ? "ON" : "OFF"); }
```

STM32용(실보드, Keil): `hal_stm32.c`
```c
#include "hal.h"
#include "stm32f10x.h"

#define LED_PIN   13u

void hal_led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));
}

void hal_led_set(uint8_t on)
{
    if (on) { GPIOC->BRR  = (1u << LED_PIN); }   /* 액티브 로우: ON=0 */
    else    { GPIOC->BSRR = (1u << LED_PIN); }   /* OFF=1 */
}
```

### 예제 4 — `pc_test.c` : 응용을 PC HAL로 구동
```c
#include "app.h"
#include <stdint.h>

/*
 * 23강 PC 검산 — 같은 app_blink_pattern 을 PC HAL(hal_pc.c)로 구동.
 * Keil 에서는 hal_pc.c 대신 hal_stm32.c 를 넣으면 동일 코드가 실보드에서 동작.
 * 빌드: gcc pc_test.c app.c hal_pc.c -o pc_test
 */

int main(void)
{
    uint8_t pattern[6] = { 1, 0, 1, 1, 0, 0 };
    app_blink_pattern(pattern, 6u);
    return 0;
}
```

## PC에서 검증하기

```
gcc pc_test.c app.c hal_pc.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
[HAL-PC] led init
[HAL-PC] LED ON
[HAL-PC] LED OFF
[HAL-PC] LED ON
[HAL-PC] LED ON
[HAL-PC] LED OFF
[HAL-PC] LED OFF
```

> Keil에서는 `hal_pc.c`를 빼고 `hal_stm32.c`와 보드용 `main.c`(예: `app_blink_pattern` 호출)를 넣으면, **같은 `app.c`** 로 PC13 LED가 패턴대로 켜집니다.

## 자주 하는 실수

### Q. HAL을 만들었는데 응용에 여전히 레지스터 코드가 남아 있어요.
A. 레지스터 접근은 **전부 HAL 구현**으로 내려야 합니다. 응용(`app.c`)에 `GPIOC->...`가 한 줄이라도 있으면 이식성이 깨집니다.

### Q. HAL 구현에서 응용 함수를 호출했어요.
A. 의존 방향 위반입니다(HAL→응용 금지). HAL은 응용을 몰라야 다른 프로젝트에 재사용됩니다. 필요하면 콜백을 인자로 받으세요.

### Q. 두 구현(hal_pc.c, hal_stm32.c)을 같이 링크하면 에러가 나요.
A. 같은 함수가 중복 정의됩니다. **한 번에 하나만** 링크하세요(PC면 hal_pc.c, 보드면 hal_stm32.c).

### Q. 추상화를 너무 많이 했더니 느리고 복잡해요.
A. 추상화는 공짜가 아닙니다. 자주 바뀌거나 이식·테스트가 필요한 경계에만 HAL을 두고, 과하게 계층을 쌓지 마세요(균형).

## 정리
- 펌웨어를 응용/HAL/하드웨어로 나누면 이식성과 테스트성이 올라간다.
- 헤더는 계약(인터페이스), 소스는 구현 — 응용은 헤더만 의존한다.
- 칩을 바꿀 땐 HAL 구현만 교체하고 응용은 그대로 둔다.
- 의존 방향은 응용→HAL 한 방향(역방향 금지).
- PC용 가짜 HAL로 보드 없이 응용 로직을 검증할 수 있다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[24. 종합 — UART 명령으로 LED·타이머 제어](../24_종합_프로젝트/README.md) — 마지막 캡스톤. 링버퍼·명령 파서·HAL·타이머를 모두 엮어, UART로 받은 명령으로 LED와 타이머를 제어하는 작은 펌웨어를 완성합니다.
