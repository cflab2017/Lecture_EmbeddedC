# 23. HAL 추상화·모듈 분리·이식성

지금까지 LED를 켜는 코드는 `GPIOC->BRR = ...`처럼 레지스터를 직접 건드렸습니다. 그런데 만약 칩을 STM32에서 8051로 바꾼다면? 또는 같은 동작을 PC에서 테스트하고 싶다면? 레지스터 코드가 응용 로직 곳곳에 박혀 있으면, 모든 곳을 다시 고쳐야 합니다. **HAL(Hardware Abstraction Layer)** 은 이 문제를 푸는 설계입니다. "무엇을 한다"(LED를 켠다)는 응용과, "어떻게 한다"(특정 레지스터를 쓴다)는 구현을 갈라놓는 것이죠.

이번 편은 같은 응용 로직(`app.c`)을 **세 가지 HAL 구현**으로 돌려 봅니다 — PC용(`hal_pc.c`, printf), STM32용(`hal_stm32.c`, 레지스터), 그리고 **아두이노 우노용(`hal_arduino.c`, ATmega328P 레지스터)**. 칩 아키텍처가 ARM에서 AVR로 완전히 바뀌어도, 응용 코드는 한 줄도 바뀌지 않습니다. 이것이 모듈 분리가 주는 이식성과 테스트 용이성입니다.

## 학습 목표
- 펌웨어를 응용/HAL/하드웨어 계층으로 나누는 이유를 설명한다.
- 인터페이스(헤더)와 구현(소스)을 분리한다.
- 같은 응용을 여러 HAL 구현으로 이식한다.
- 의존 방향(응용→HAL, 역방향 금지)을 지킨다.
- PC용 가짜 HAL로 응용 로직을 단위 테스트한다.

## 대상 환경
- 컴파일러: **PC + gcc**(응용+PC HAL) / Keil MDK(응용+STM32 HAL) / **avr-gcc·Arduino IDE**(응용+Arduino HAL).
- 디바이스: 없음(응용·HAL 인터페이스는 비의존). STM32 HAL은 STM32F103C8, Arduino HAL은 ATmega328P(우노).
- 검증: PC `gcc`(`pc_test.c` + `app.c` + `hal_pc.c`). 보드용은 각 툴체인에서 빌드.

> 주의: 아두이노 우노는 **AVR(ATmega328P)** 이라 **Keil로는 빌드할 수 없습니다**(Keil은 8051·ARM 전용). 우노용 `hal_arduino.c`는 avr-gcc 또는 Arduino IDE로 빌드합니다. 반면 순수 모듈(`app.c`)과 PC HAL은 그대로 재사용됩니다.

## 핵심 개념

### 1) 계층화(layering)
펌웨어를 책임에 따라 층으로 나눕니다. 위층은 아래층만 호출하고, 아래층은 위층을 모릅니다.

```
┌────────────────────┐
│ 응용 (app.c)        │  "패턴대로 LED 제어" — 하드웨어 모름
├────────────────────┤
│ HAL 인터페이스(hal.h)│  "led_set(on)" 같은 계약
├────────────────────┤
│ HAL 구현            │  hal_pc.c / hal_stm32.c / hal_arduino.c
├────────────────────┤
│ 하드웨어            │  PC stdout / STM32 GPIO / AVR GPIO
└────────────────────┘
```

### 2) 인터페이스와 구현의 분리
헤더(`hal.h`)는 **계약**입니다 — 함수 이름·인자·의미만 정합니다. 소스(`hal_pc.c`, `hal_stm32.c`)는 그 계약을 각자의 방식으로 **구현**합니다. 응용은 헤더만 `#include`하고 어떤 구현이 링크될지는 신경 쓰지 않습니다.

### 3) 이식성 — 구현만 교체
```
PC 테스트:   pc_test.c + app.c + hal_pc.c       → printf 로 동작 확인
STM32 보드:  main.c    + app.c + hal_stm32.c    → PC13 LED 동작 (ARM)
우노 보드:   main(.ino)+ app.c + hal_arduino.c  → PB5(D13) LED 동작 (AVR)
```
`app.c`는 셋 모두에서 **글자 단위로 동일**합니다. 칩이 ARM이든 AVR이든, 바꾸는 것은 HAL 구현 파일 하나뿐입니다.

### 4) 의존 방향 규칙
- 응용 → HAL : 허용 (위가 아래를 호출)
- HAL → 응용 : **금지** (아래가 위를 알면 분리가 깨진다)

이 방향을 지켜야 HAL을 다른 프로젝트에 떼어다 재사용할 수 있습니다.

### 5) 테스트 용이성
하드웨어가 없어도, PC용 가짜 HAL을 끼우면 응용 로직을 `gcc`로 즉시 검증할 수 있습니다. 로직 버그를 보드 없이 빠르게 잡는 것이 큰 이점입니다(이 트랙의 `pc_test.c`가 바로 이 방식).

### 6) 아두이노 우노(ATmega328P) 레지스터 매핑
아두이노 우노의 칩은 **AVR ATmega328P**(8비트)입니다. ARM·8051과 또 다른 아키텍처지만, GPIO를 레지스터로 다루는 원리는 같습니다. 우노 온보드 LED는 **'디지털 13번' = PB5**(포트 B의 비트 5)이며 **액티브 하이**(핀=1일 때 켜짐)입니다.

| 동작 | AVR 레지스터 | STM32 대응(비교) |
|------|--------------|------------------|
| 핀을 출력으로 | `DDRB |= (1<<5)` | `GPIOC->CRH` 모드 비트 |
| 출력 High(켜기) | `PORTB |= (1<<5)` | `GPIOC->BSRR`/`BRR` |
| 출력 Low(끄기) | `PORTB &= ~(1<<5)` | `GPIOC->BSRR`/`BRR` |

`DDRx`(방향)·`PORTx`(출력)·`PINx`(입력)가 AVR GPIO의 3대 레지스터입니다. `digitalWrite()` 같은 Arduino API를 쓰지 않고 이 레지스터를 직접 쓰면, 이 트랙의 '레지스터 직접 제어' 철학을 AVR에서도 유지하면서 같은 `hal.h` 계약을 구현할 수 있습니다.

> 핵심: 아키텍처가 ARM→AVR로 통째로 바뀌어도 **응용(`app.c`)은 그대로**고, 새로 쓰는 것은 `hal_arduino.c` 한 파일뿐입니다.

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

### 예제 3 — 세 가지 HAL 구현
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

아두이노 우노용(실보드, avr-gcc/Arduino IDE): `hal_arduino.c`
```c
#include "hal.h"
#include <avr/io.h>

/*
 * HAL 의 Arduino Uno(ATmega328P) 구현 — AVR 레지스터 직접 제어.
 * 우노 온보드 LED 는 '디지털 13번' = PB5, 액티브 하이(1=켜짐).
 * digitalWrite 같은 Arduino API 를 쓰지 않고 DDRB/PORTB 를 직접 다룬다.
 */

#define LED_BIT   5u    /* PB5 = 디지털 13 */

void hal_led_init(void)
{
    DDRB |= (uint8_t)(1u << LED_BIT);     /* PB5 를 출력으로 */
}

void hal_led_set(uint8_t on)
{
    if (on) {
        PORTB |= (uint8_t)(1u << LED_BIT);    /* 액티브 하이: ON = 1 */
    } else {
        PORTB &= (uint8_t)~(1u << LED_BIT);   /* OFF = 0 */
    }
}
```

세 구현 모두 같은 `hal.h`를 만족합니다. `hal_led_set` 한 줄을 PC는 printf로, STM32는 `BRR/BSRR`로, 우노는 `PORTB`로 — 칩에 맞게만 바꿉니다.

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

## 아두이노 우노에서 실행하기
우노는 AVR이라 Keil이 아니라 **avr-gcc / Arduino IDE**로 빌드합니다. `app.c` + `hal_arduino.c`를 그대로 쓰고, 진입점만 우노용으로 둡니다(패턴이 눈에 보이도록 지연을 넣은 예).

```c
/* arduino_demo: app.c + hal.h + hal_arduino.c 와 함께 빌드 */
#define F_CPU 16000000UL          /* 우노 16MHz */
#include <util/delay.h>
#include "app.h"
#include "hal.h"

int main(void)
{
    uint8_t pattern[6] = { 1, 0, 1, 1, 0, 0 };
    while (1) {
        uint8_t i;
        hal_led_init();
        for (i = 0; i < 6; i++) {
            hal_led_set(pattern[i]);
            _delay_ms(300);       /* 사람이 볼 수 있게 */
        }
    }
}
```

- **avr-gcc 빌드 예**:
  `avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os arduino_demo.c app.c hal_arduino.c -o blink.elf`
  → `avr-objcopy -O ihex blink.elf blink.hex` → `avrdude`로 업로드.
- **Arduino IDE**: `.ino` 스케치에서 같은 로직을 `main` 대신 `setup()/loop()`로 옮기고, `app.c`·`hal_arduino.c`를 스케치 폴더에 함께 둡니다(레지스터 직접 제어는 Arduino IDE에서도 그대로 동작).
- **시뮬레이션**: 보드가 없으면 [Wokwi](https://wokwi.com)·SimulIDE 같은 우노 시뮬레이터에서 D13 LED 점멸을 확인할 수 있습니다.

> 핵심 메시지: PC→STM32(ARM)→우노(AVR)로 무대가 바뀌어도 `app.c`는 단 한 글자도 바뀌지 않았습니다. 잘 그은 HAL 경계 하나가 이식성을 만듭니다.

## 자주 하는 실수

### Q. HAL을 만들었는데 응용에 여전히 레지스터 코드가 남아 있어요.
A. 레지스터 접근은 **전부 HAL 구현**으로 내려야 합니다. 응용(`app.c`)에 `GPIOC->...`가 한 줄이라도 있으면 이식성이 깨집니다.

### Q. HAL 구현에서 응용 함수를 호출했어요.
A. 의존 방향 위반입니다(HAL→응용 금지). HAL은 응용을 몰라야 다른 프로젝트에 재사용됩니다. 필요하면 콜백을 인자로 받으세요.

### Q. 여러 구현(hal_pc.c, hal_stm32.c, hal_arduino.c)을 같이 링크하면 에러가 나요.
A. 같은 함수가 중복 정의됩니다. **한 번에 하나만** 링크하세요(PC면 hal_pc.c, STM32면 hal_stm32.c, 우노면 hal_arduino.c).

### Q. 우노 코드를 Keil에서 빌드하려는데 안 돼요.
A. 아두이노 우노는 AVR(ATmega328P)이라 **Keil이 지원하지 않습니다**(Keil은 8051·ARM 전용). 우노용은 avr-gcc 또는 Arduino IDE로 빌드하세요. `app.c`·`hal.h`는 그대로 쓰고 `hal_arduino.c`만 추가합니다.

### Q. 추상화를 너무 많이 했더니 느리고 복잡해요.
A. 추상화는 공짜가 아닙니다. 자주 바뀌거나 이식·테스트가 필요한 경계에만 HAL을 두고, 과하게 계층을 쌓지 마세요(균형).

## 정리
- 펌웨어를 응용/HAL/하드웨어로 나누면 이식성과 테스트성이 올라간다.
- 헤더는 계약(인터페이스), 소스는 구현 — 응용은 헤더만 의존한다.
- 칩을 바꿀 땐 HAL 구현만 교체하고 응용은 그대로 둔다.
- 같은 `app.c`가 PC·STM32(ARM)·우노(AVR) 세 구현에서 그대로 동작한다.
- 의존 방향은 응용→HAL 한 방향(역방향 금지).
- PC용 가짜 HAL로 보드 없이 응용 로직을 검증할 수 있다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[24. 종합 — UART 명령으로 LED·타이머 제어](../24_종합_프로젝트/README.md) — 마지막 캡스톤. 링버퍼·명령 파서·HAL·타이머를 모두 엮어, UART로 받은 명령으로 LED와 타이머를 제어하는 작은 펌웨어를 완성합니다.
