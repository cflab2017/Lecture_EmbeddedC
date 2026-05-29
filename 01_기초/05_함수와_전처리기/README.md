# 05. 함수·헤더 분리·#define 매크로

코드가 길어지면 한 파일에 다 몰아넣을 수 없습니다. 펌웨어는 보통 "GPIO 다루는 부분", "UART 다루는 부분", "응용 로직" 식으로 **모듈**로 나눕니다. 모듈을 나누는 도구가 바로 **함수**(동작 단위), **헤더(.h)와 소스(.c) 분리**(인터페이스/구현), 그리고 **전처리기**(`#include`, `#define`, 헤더 가드)입니다.

이번 편은 기초의 마지막으로, 비트 조작 로직을 `ledbank` 모듈(`ledbank.h` + `ledbank.c`)로 떼어내고 `main.c`/`pc_test.c`가 그 인터페이스만 보고 쓰는 구조를 만듭니다. 헤더 가드와 매크로의 함정(괄호!)도 함께 다룹니다. 모듈로 나누는 감각은 23편(HAL 추상화)까지 이어지는, 펌웨어를 키우는 핵심 습관입니다.

## 학습 목표
- 함수 선언(프로토타입)과 정의를 구분하고, 동작을 함수로 분리한다.
- 헤더(.h, 인터페이스)와 소스(.c, 구현)를 나누고, 여러 파일을 함께 빌드한다.
- 헤더 가드(`#ifndef`/`#define`/`#endif`)로 중복 포함을 막는다.
- `#define` 상수와 함수형 매크로를 구분하고, 매크로 괄호 함정을 피한다.
- 조건부 컴파일(`#ifdef`)로 플랫폼/옵션을 분기하는 개념을 안다.

## 대상 환경
- 컴파일러: Keil C51 / Keil MDK-ARM 중 무엇이든 (이번 편은 순수 로직).
- 디바이스: 순수 로직 — **PC + gcc 또는 μVision 시뮬레이터** (실제 보드 불필요).
- 검증: PC의 `gcc`(`main.c`/`pc_test.c` + `ledbank.c`).

## 핵심 개념

### 1) 함수 — 선언과 정의
함수는 동작에 이름을 붙여 재사용·테스트하기 쉽게 만듭니다.

- **선언(프로토타입)**: 이름·반환형·인자형만 약속. `uint8_t led_set(uint8_t bank, uint8_t pos);`
- **정의**: 실제 몸체. `uint8_t led_set(...) { return ...; }`

선언을 헤더에, 정의를 소스에 두면, 다른 파일은 헤더만 보고 함수를 호출할 수 있습니다(구현은 몰라도 됨).

### 2) 헤더(.h)와 소스(.c) 분리
| 파일 | 역할 | 담는 것 |
|------|------|---------|
| `ledbank.h` | 인터페이스 | `#define`, 타입, 함수 **선언** |
| `ledbank.c` | 구현 | 함수 **정의**(몸체) |
| `main.c` | 사용 | `#include "ledbank.h"` 후 함수 호출 |

빌드할 때는 사용하는 `.c`들을 함께 컴파일합니다.

```
gcc main.c ledbank.c -o main
```

μVision에서는 프로젝트에 `main.c`와 `ledbank.c`를 모두 **Source Group**에 추가하면 됩니다(`.h`는 추가하지 않아도 `#include`로 찾습니다).

### 3) 헤더 가드 — 중복 포함 방지
같은 헤더가 여러 경로로 두 번 포함되면 타입·매크로가 중복 정의되어 오류가 납니다. 이를 막는 관용구가 헤더 가드입니다.

```c
#ifndef LEDBANK_H      /* 아직 정의 안 됐으면 */
#define LEDBANK_H      /* 정의 표시하고 */
... 헤더 내용 ...
#endif /* LEDBANK_H */ /* 두 번째부터는 통째로 건너뜀 */
```

`#pragma once`도 같은 일을 하지만, 모든 컴파일러가 보장하지 않으므로 이식성을 위해 `#ifndef` 가드를 권합니다.

### 4) `#define` — 상수 매크로 vs 함수형 매크로
전처리기는 컴파일 전에 **텍스트를 치환**합니다.

```c
#define LED_COUNT  8u            /* 상수 매크로 */
#define BIT(n)     (1u << (n))   /* 함수형 매크로 */
```

함수형 매크로에서 **인자와 전체 식을 모두 괄호로** 감싸는 것이 절대 규칙입니다. 안 그러면 우선순위 때문에 깨집니다.

```c
#define SQ(x) x*x          /* 나쁨: SQ(a+1) → a+1*a+1 (= a + a + 1) */
#define SQ(x) ((x)*(x))    /* 좋음: SQ(a+1) → ((a+1)*(a+1)) */
```

매크로는 타입 검사가 없고 인자를 여러 번 평가하므로(`SQ(i++)` 위험), 단순 상수·간단한 식이 아니면 `static` 함수(또는 `inline`)가 더 안전합니다.

### 5) 조건부 컴파일 — `#ifdef`
플랫폼/옵션에 따라 코드를 골라 컴파일합니다. 같은 소스로 8051·STM32·PC를 분기할 때 씁니다.

```c
#ifdef USE_UART
    uart_send(msg);     /* UART 빌드에서만 포함 */
#else
    /* 아무것도 안 함 */
#endif
```

## μVision 으로 보기 (이번 편은 PC로도 충분)
순수 로직이라 PC로 충분합니다. μVision에서는 `main.c`와 `ledbank.c`를 프로젝트에 추가하고 F7로 빌드하면 두 파일이 함께 링크됩니다. 모듈 분리는 어느 IDE에서나 동일한 방식입니다.

## 예제로 보기

### 예제 1 — `ledbank.h` / `ledbank.c` / `main.c` : 모듈 분리
비트 조작을 `ledbank` 모듈로 떼어내고, `main`은 함수 이름만으로 의도를 표현합니다.

`ledbank.h` (인터페이스):

```c
#ifndef LEDBANK_H
#define LEDBANK_H

#include <stdint.h>

/*
 * ledbank.h — 8개 LED 뱅크(8비트)를 다루는 모듈의 '인터페이스'.
 * 구현은 ledbank.c 에 있고, 사용자는 이 헤더만 #include 하면 된다.
 *
 * 헤더 가드(#ifndef ~ #define ~ #endif): 같은 헤더가 여러 번 포함돼도
 * 내용이 한 번만 처리되게 막는다. 빠뜨리면 중복 정의 오류가 난다.
 */

/* 함수형 매크로 — 인자는 반드시 괄호로 감싼다(우선순위 사고 방지). */
#define BIT(n)       (1u << (n))
#define LED_COUNT    8u

/* 함수 선언(프로토타입). 반환·인자 타입만 약속하고 구현은 .c 에 둔다. */
uint8_t led_set(uint8_t bank, uint8_t pos);
uint8_t led_clear(uint8_t bank, uint8_t pos);
uint8_t led_toggle(uint8_t bank, uint8_t pos);
uint8_t led_count_on(uint8_t bank);

#endif /* LEDBANK_H */
```

`ledbank.c` (구현):

```c
#include "ledbank.h"

/*
 * ledbank.c — ledbank.h 가 선언한 함수들의 '구현'.
 * 비트 조작 관용구(3편)를 의미 있는 이름의 함수로 감쌌다.
 */

uint8_t led_set(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank | BIT(pos));
}

uint8_t led_clear(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank & ~BIT(pos));
}

uint8_t led_toggle(uint8_t bank, uint8_t pos)
{
    return (uint8_t)(bank ^ BIT(pos));
}

uint8_t led_count_on(uint8_t bank)
{
    uint8_t pos;
    uint8_t count = 0;
    for (pos = 0; pos < LED_COUNT; pos++) {
        count = (uint8_t)(count + ((bank >> pos) & 1u));
    }
    return count;
}
```

`main.c` (사용):

```c
#include <stdio.h>
#include "ledbank.h"   /* 우리 모듈의 인터페이스 */

/* 8비트 값을 2진수 8자리로 출력한다. */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

/*
 * 05강 예제 1 — 모듈(ledbank)을 가져다 쓰는 쪽.
 * main 은 비트 연산의 '방법'을 몰라도, 함수 이름만으로 의도를 표현한다.
 * 빌드: gcc main.c ledbank.c -o main
 */
int main(void)
{
    uint8_t bank = 0x00;

    bank = led_set(bank, 1);
    bank = led_set(bank, 4);
    bank = led_set(bank, 6);
    bank = led_toggle(bank, 4);   /* 4번은 켰다가 다시 끔 */

    printf("bank   = ");
    print_bin8(bank);
    printf("\n");
    printf("LED on = %u\n", (unsigned)led_count_on(bank));

    return 0;
}
```

### 예제 2 — `pc_test.c` : 모듈만 떼어 검증
같은 모듈을 테스트 하니스로 검증합니다.

```c
#include <stdio.h>
#include "ledbank.h"

/*
 * 05강 예제 2 — ledbank 모듈 함수들을 단위 검증한다.
 * 하드웨어 없이 PC 에서 모듈만 떼어 테스트할 수 있다.
 * 빌드: gcc pc_test.c ledbank.c -o pc_test
 */
int main(void)
{
    uint8_t b = 0x00;

    b = led_set(b, 0);
    b = led_set(b, 3);
    printf("set 0,3   = 0x%02X\n", b);     /* 0x09 */

    b = led_clear(b, 0);
    printf("clear 0   = 0x%02X\n", b);     /* 0x08 */

    b = led_toggle(b, 7);
    printf("toggle 7  = 0x%02X\n", b);     /* 0x88 */

    printf("count on  = %u\n", (unsigned)led_count_on(b));  /* 2 */

    return 0;
}
```

## PC에서 검증하기
이번 편은 모듈이 나뉘어 있으니 사용하는 `.c`를 함께 컴파일합니다.

```
gcc main.c ledbank.c -o main && ./main
```

예상 출력(stdout):

```
bank   = 01000010
LED on = 2
```

```
gcc pc_test.c ledbank.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
set 0,3   = 0x09
clear 0   = 0x08
toggle 7  = 0x88
count on  = 2
```

## 자주 하는 실수

### Q. `multiple definition` / `redefinition` 오류가 나요.
A. 헤더에 함수 **정의(몸체)** 를 넣고 여러 .c에서 포함하면 정의가 여러 번 생깁니다. 헤더에는 **선언**만, 정의는 .c에 두세요. 또 헤더 가드(`#ifndef`)를 빠뜨리면 한 .c 안에서도 중복 포함으로 오류가 납니다.

### Q. `undefined reference to led_set` 링크 오류가 나요.
A. `ledbank.c`를 함께 빌드하지 않았기 때문입니다. 헤더는 "선언"만 주므로, 구현이 든 `.c`를 같이 컴파일/링크해야 합니다: `gcc main.c ledbank.c -o main`. μVision이라면 두 .c를 모두 프로젝트에 추가하세요.

### Q. `#define SQ(x) x*x`로 만든 매크로가 이상한 값을 줘요.
A. 괄호가 없어서입니다. `SQ(a+1)`이 `a+1*a+1`로 전개됩니다. 함수형 매크로는 인자와 전체 식을 모두 괄호로: `#define SQ(x) ((x)*(x))`. 부작용 있는 인자(`SQ(i++)`)는 여러 번 평가되니 함수를 쓰세요.

### Q. 상수에 `#define`과 `const` 중 뭘 쓰죠?
A. 둘 다 가능합니다. `#define`은 전처리 치환이라 타입이 없고 디버거에서 안 보일 수 있습니다. `const`는 타입이 있고 디버깅에 유리합니다. 비트 위치/주소처럼 전처리 단계에서 필요한 값은 `#define`, 일반 상수는 `const`도 좋은 선택입니다.

## 정리
- 동작은 함수로 나누고, 선언은 헤더(.h)에, 정의는 소스(.c)에 둔다.
- 사용하는 .c들을 함께 빌드해야 링크된다(`gcc main.c ledbank.c`).
- 헤더 가드(`#ifndef/#define/#endif`)로 중복 포함을 막는다.
- 함수형 매크로는 인자·식을 모두 괄호로 감싼다. 복잡하면 함수가 안전하다.
- `#ifdef`로 플랫폼/옵션별 코드를 골라 컴파일한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[06. 8051 구조·SFR·reg52.h](../../02_8051/06_8051_구조와_SFR/README.md) — 드디어 실제 칩입니다. 8051의 구조와 특수 기능 레지스터(SFR), `reg52.h`로 포트를 직접 다루기 시작합니다.
