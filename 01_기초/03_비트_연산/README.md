# 03. 비트 연산과 레지스터 조작 (set/clear/toggle)

임베디드에서 하드웨어를 다룬다는 건 결국 **레지스터의 특정 비트를 켜고·끄고·읽는** 일입니다. "GPIO 3번 핀을 High로", "타이머 인터럽트 플래그를 클리어", "UART 송신 완료 비트가 1이 될 때까지 대기" — 모두 한 레지스터 안의 **개별 비트 하나**를 다루는 작업이죠. 이때 나머지 비트는 절대 건드리면 안 됩니다. 옆 비트가 다른 핀·다른 기능을 제어하고 있을 수 있으니까요.

이번 편은 그 핵심 도구인 **비트 연산자**(`& | ^ ~ << >>`)와, 이를 조합한 세 가지 황금 관용구 **set / clear / toggle**, 그리고 비트를 읽는 **test**를 익힙니다. 보드 없이 8비트 변수를 "포트 레지스터"라 가정하고 PC의 `gcc`로 결과를 눈으로 확인합니다. 여기서 익힌 관용구는 6편 이후 8051·STM32 레지스터 제어에서 그대로 다시 쓰입니다.

## 학습 목표
- 비트 연산자 6종(`&`, `|`, `^`, `~`, `<<`, `>>`)의 동작을 진리표/예시로 설명한다.
- 비트 마스크 `(1u << n)`를 만들어 원하는 비트만 지정한다.
- 레지스터의 한 비트를 set(`|=`)·clear(`&= ~`)·toggle(`^=`)하고, test(`>> &`)로 읽는다.
- 나머지 비트를 보존하는 read-modify-write의 의미를 이해한다.
- 여러 비트를 마스크로 한꺼번에 set/clear 한다.

## 대상 환경
- 컴파일러: Keil C51 / Keil MDK-ARM 중 무엇이든 (이번 편은 순수 로직).
- 디바이스: 순수 로직 — **PC + gcc 또는 μVision 시뮬레이터** (실제 보드 불필요).
- 검증: PC의 `gcc`(`main.c`, `pc_test.c`).

## 핵심 개념

### 1) 비트 연산자 6종
비트 연산자는 정수를 비트의 나열로 보고 자리마다 연산합니다.

| 연산자 | 이름 | 규칙 | 용도 |
|--------|------|------|------|
| `&` | AND | 둘 다 1이어야 1 | 비트 끄기/검사(마스킹) |
| `\|` | OR | 하나라도 1이면 1 | 비트 켜기 |
| `^` | XOR | 다르면 1 | 비트 뒤집기(토글) |
| `~` | NOT | 0↔1 반전 | 마스크 반전 |
| `<<` | 왼쪽 시프트 | 비트를 왼쪽으로 | 마스크 생성, ×2ⁿ |
| `>>` | 오른쪽 시프트 | 비트를 오른쪽으로 | 비트 추출, ÷2ⁿ |

> 주의: `&`/`|`/`^`는 **비트** 연산이고, `&&`/`||`는 **논리** 연산(참/거짓)입니다. 비트를 다룰 때 `&&`를 쓰면 전혀 다른 결과가 나옵니다.

### 2) 비트 마스크 — `(1u << n)`
n번 비트만 1인 값을 **마스크**라 합니다. `1u`(비트 0이 1)를 왼쪽으로 n칸 밀어 만듭니다.

```
1u << 0  = 00000001   (bit 0)
1u << 3  = 00001000   (bit 3)
1u << 7  = 10000000   (bit 7)
```

매직 넘버를 피하려고 비트 위치에 이름을 붙입니다: `#define LED_RED 0u` → `BIT(LED_RED)`.

### 3) 황금 관용구 — set / clear / toggle
한 비트만 조작하면서 **나머지 비트는 그대로 두는** 것이 핵심입니다(read-modify-write).

| 동작 | 코드 | 원리 |
|------|------|------|
| set (1로) | `reg \|= (1u << n);` | OR 1 → 그 비트만 1, 나머지 불변 |
| clear (0으로) | `reg &= ~(1u << n);` | AND 0 → 그 비트만 0, 나머지는 AND 1로 불변 |
| toggle (반전) | `reg ^= (1u << n);` | XOR 1 → 그 비트만 반전, 나머지 불변 |

`~`(NOT)가 clear의 핵심입니다. `~(1u<<n)`은 n번만 0이고 나머지는 전부 1인 마스크라, AND하면 n번만 0이 되고 나머지는 보존됩니다.

### 4) 비트 읽기 — test
특정 비트가 켜졌는지 확인합니다. 두 가지 방법이 있습니다.

```c
if ((reg >> n) & 1u) { ... }      /* 그 비트를 LSB로 내려 0/1 로 */
if (reg & (1u << n)) { ... }      /* 마스킹: 0 또는 (1<<n) → 참/거짓 */
```

둘 다 흔히 쓰지만, **정확히 0 또는 1**이 필요하면 `(reg >> n) & 1u`가 안전합니다. `reg & (1u<<n)`은 참일 때 `1<<n`(예: 0x80) 값이라, 1과 직접 비교(`== 1`)하면 어긋납니다.

### 5) 여러 비트를 한꺼번에
마스크에 여러 비트를 OR로 묶으면 동시에 처리할 수 있습니다.

```c
#define LOWER_NIBBLE 0x0Fu          /* 하위 4비트 */
reg |=  LOWER_NIBBLE;               /* 하위 4비트 모두 set */
reg &= ~LOWER_NIBBLE;               /* 하위 4비트 모두 clear */
reg &= ~(BIT(1) | BIT(3));          /* bit1, bit3 만 clear */
```

## μVision 으로 보기 (이번 편은 PC로도 충분)
순수 로직 편이라 특정 디바이스가 필요 없습니다. μVision 디버그에서 `reg`를 Watch 창에 넣고 Number Base를 Binary/Hex로 바꿔 각 단계의 비트 변화를 관찰할 수 있습니다. 정식 GPIO 레지스터 제어는 7편(8051)·14편(STM32)에서 다룹니다.

## 예제로 보기

### 예제 1 — `main.c` : 8비트 포트 레지스터 조작
8비트 변수를 "LED 포트"로 보고 set/clear/toggle/test를 순서대로 적용하며 매 단계의 비트를 출력합니다.

```c
#include <stdio.h>
#include <stdint.h>

/* 비트 위치에 의미 있는 이름을 준다 (매직 넘버 금지). */
#define LED_RED     0u   /* bit 0 */
#define LED_GREEN   1u   /* bit 1 */
#define LED_BLUE    2u   /* bit 2 */

#define BIT(n)      (1u << (n))   /* n 번 비트만 1 인 마스크 */

/* 8비트 값을 2진수 8자리로 출력한다. */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

/* 라벨과 현재 레지스터 상태(2진)를 한 줄로 출력한다. */
static void show(const char *label, uint8_t reg)
{
    printf("%-12s: ", label);
    print_bin8(reg);
    printf("\n");
}

/*
 * 03강 예제 1 — 8비트 '포트' 레지스터를 비트 단위로 조작한다.
 * 실제 보드라면 reg 가 GPIO 출력 레지스터이고, 각 비트가 LED 한 개다.
 * set( |= ), clear( &= ~ ), toggle( ^= ), test( >> & ) 네 관용구를 본다.
 */
int main(void)
{
    uint8_t reg = 0x00;            /* 모든 LED OFF 로 시작 */

    show("init", reg);

    reg |= BIT(LED_RED);           /* set   : RED 켜기 */
    show("set RED", reg);

    reg |= BIT(LED_BLUE);          /* set   : BLUE 켜기 */
    show("set BLUE", reg);

    reg ^= BIT(LED_RED);           /* toggle: RED 뒤집기(켜져 있으니 꺼짐) */
    show("toggle RED", reg);

    reg &= (uint8_t)~BIT(LED_BLUE);/* clear : BLUE 끄기 */
    show("clear BLUE", reg);

    /* test: GREEN 비트가 켜져 있는가? */
    printf("GREEN is %s\n", ((reg >> LED_GREEN) & 1u) ? "ON" : "OFF");

    return 0;
}
```

### 예제 2 — `pc_test.c` : 관용구를 함수로 분리해 검증
set/clear/toggle/test를 순수 함수로 떼어내면 하드웨어 없이 단위 검증할 수 있습니다.

```c
#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/* 순수 로직: 레지스터의 n 번 비트를 set/clear/toggle/test 한다.
 * 하드웨어 의존이 없어 PC 에서 그대로 검증할 수 있다. */
static uint8_t set_bit(uint8_t reg, unsigned n)    { return (uint8_t)(reg | BIT(n)); }
static uint8_t clear_bit(uint8_t reg, unsigned n)  { return (uint8_t)(reg & ~BIT(n)); }
static uint8_t toggle_bit(uint8_t reg, unsigned n) { return (uint8_t)(reg ^ BIT(n)); }
static int     test_bit(uint8_t reg, unsigned n)   { return (int)((reg >> n) & 1u); }

/*
 * 03강 예제 2 — set/clear/toggle/test 관용구를 함수로 떼어내 검증한다.
 */
int main(void)
{
    uint8_t reg = 0x00;

    reg = set_bit(reg, 0);          /* 0000 0001 */
    reg = set_bit(reg, 7);          /* 1000 0001 */
    printf("after set 0,7  = 0x%02X\n", reg);

    reg = clear_bit(reg, 0);        /* 1000 0000 */
    printf("after clear 0  = 0x%02X\n", reg);

    reg = toggle_bit(reg, 1);       /* 1000 0010 */
    printf("after toggle 1 = 0x%02X\n", reg);

    printf("bit7 = %d, bit6 = %d\n", test_bit(reg, 7), test_bit(reg, 6));

    return 0;
}
```

## PC에서 검증하기

```
gcc main.c -o main && ./main
```

예상 출력(stdout):

```
init        : 00000000
set RED     : 00000001
set BLUE    : 00000101
toggle RED  : 00000100
clear BLUE  : 00000000
GREEN is OFF
```

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
after set 0,7  = 0x81
after clear 0  = 0x80
after toggle 1 = 0x82
bit7 = 1, bit6 = 0
```

## 자주 하는 실수

### Q. 한 비트만 끄려고 `reg &= (1u << n);` 했더니 다른 비트가 다 사라졌어요.
A. clear는 **반전 마스크**가 필요합니다. `(1u<<n)`은 그 비트만 1이라, AND하면 그 비트만 남기고 나머지를 전부 0으로 지웁니다. 올바른 형태는 `reg &= ~(1u << n);` — `~`로 "그 비트만 0, 나머지 1" 마스크를 만들어야 합니다.

### Q. `reg ^= (1u<<n)` 대신 `reg |= (1u<<n)`로 토글하려는데 안 돼요.
A. OR(`|`)는 비트를 항상 1로 만들 뿐, 다시 0으로 되돌리지 못합니다. 켰다 껐다 반복하는 토글은 XOR(`^`)입니다. 같은 비트에 XOR 1을 두 번 하면 원래대로 돌아옵니다.

### Q. `if (reg & (1u<<7) == 1)`이 항상 거짓이에요.
A. 두 가지 함정입니다. ① `==`가 `&`보다 우선순위가 높아 `(1u<<7)==1`(거짓=0)이 먼저 계산됩니다 → 괄호 필요: `if ((reg & (1u<<7)) ...)`. ② `reg & (1u<<7)`은 참일 때 `0x80`이라 `== 1`과 절대 같지 않습니다. `if ((reg >> 7) & 1u)` 또는 `if (reg & (1u<<7))`로 쓰세요.

### Q. `1 << 31` 같은 큰 시프트가 이상해요.
A. `1`은 `int`라 부호 비트까지 밀면 미정의 동작입니다. 마스크는 항상 부호 없는 리터럴 `1u`(필요하면 `1ul`/`1UL`)로 시작하세요. 32비트 레지스터(STM32)에서는 `1u << 31`이 안전합니다.

## 정리
- 비트 마스크는 `(1u << n)` — n번 비트만 1.
- set은 `|=`, clear는 `&= ~`, toggle은 `^=`. 세 관용구가 레지스터 제어의 기본.
- clear의 `~`, 시프트의 `1u`(부호 없는 리터럴)를 빠뜨리지 않는다.
- read-modify-write로 한 비트를 다루면서 나머지 비트는 보존한다.
- 비트 읽기는 `(reg >> n) & 1u`가 정확히 0/1을 준다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[04. 포인터·volatile·메모리 맵드 I/O 개념](../04_포인터와_메모리맵/README.md) — 레지스터가 결국 "특정 주소의 메모리"임을 이해하고, 포인터와 `volatile`로 그 주소에 직접 접근하는 법을 배웁니다.
