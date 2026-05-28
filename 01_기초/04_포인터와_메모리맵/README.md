# 04. 포인터·volatile·메모리 맵드 I/O 개념

3편에서 "레지스터의 비트를 조작"했습니다. 그런데 그 레지스터는 **어디에** 있을까요? 임베디드에서 레지스터는 곧 **메모리의 특정 주소**입니다. 예를 들어 STM32의 GPIOC 출력 레지스터(ODR)는 주소 `0x4001100C`에 박혀 있습니다. 그 주소에 값을 쓰면 핀이 움직이고, 그 주소를 읽으면 핀 상태가 들어옵니다. 이렇게 "주변장치를 메모리 주소처럼 다루는 것"을 **메모리 맵드 I/O(MMIO)** 라고 합니다.

주소를 직접 다루려면 **포인터**가 필요하고, 하드웨어가 멋대로 바꾸는 값을 안전하게 읽으려면 **`volatile`** 이 필요합니다. 이번 편은 이 두 개념을 PC의 `gcc`로 흉내 내며 익힙니다. 실제 주소(`0x40011000` 등)는 PC에서 접근할 수 없으니, 변수 하나를 "가짜 레지스터"로 삼아 똑같은 포인터 패턴을 연습합니다. 여기서 익히는 `(*(volatile uint32_t *)주소)` 패턴은 14편 이후 STM32 레지스터 제어의 토대입니다.

## 학습 목표
- 포인터의 주소(`&`)와 역참조(`*`)를 구분해 사용한다.
- 레지스터가 "고정 주소의 메모리"임을 이해하고, MMIO 개념을 설명한다.
- `(*(volatile uint32_t *)ADDR)` 패턴으로 특정 주소에 읽고 쓴다.
- `volatile`이 왜 필요한지(컴파일러 최적화로 인한 읽기 생략 방지) 설명한다.
- 주변장치를 구조체+포인터로 묶는 CMSIS 스타일 접근을 이해한다.

## 대상 환경
- 컴파일러: Keil C51 / Keil MDK-ARM 중 무엇이든 (이번 편은 순수 로직).
- 디바이스: 순수 로직 — **PC + gcc 또는 μVision 시뮬레이터** (실제 보드 불필요).
- 검증: PC의 `gcc`(`main.c`, `pc_test.c`).

## 핵심 개념

### 1) 포인터 — 주소를 담는 변수
포인터는 "값"이 아니라 "값이 있는 **주소**"를 담습니다.

```c
uint8_t  x = 10;
uint8_t *p = &x;   /* p 는 x 의 주소 */
*p = 20;           /* 역참조: p 가 가리키는 곳(x)에 20 을 쓴다 → x == 20 */
```

- `&x` : x의 주소.
- `*p` : p가 가리키는 곳의 값(역참조).
- `p`와 `*p`를 혼동하지 않는 것이 핵심입니다. `p`는 주소, `*p`는 그 주소의 내용물.

### 2) 메모리 맵드 I/O(MMIO) — 레지스터 = 주소
MCU에서 주변장치 레지스터는 메모리 주소 공간의 특정 번지에 배치됩니다. CPU 입장에서는 RAM에 쓰는 것과 똑같이 그 주소에 값을 쓰지만, 그 주소는 RAM이 아니라 하드웨어(핀, 타이머, UART)에 연결돼 있습니다.

| 예 (STM32F103) | 주소 | 의미 |
|----------------|------|------|
| `RCC->APB2ENR` | `0x40021018` | 주변장치 클럭 인에이블 |
| `GPIOC->CRH` | `0x40011004` | 포트C 상위 핀 설정 |
| `GPIOC->ODR` | `0x4001100C` | 포트C 출력 데이터 |

그래서 레지스터 접근은 "그 주소를 가리키는 포인터를 역참조"하는 일입니다.

```c
#define GPIOC_ODR (*(volatile uint32_t *)0x4001100CUL)
GPIOC_ODR |= (1u << 13);   /* PC13 출력 High */
```

`(volatile uint32_t *)0x4001100CUL`은 "주소 `0x4001100C`를 32비트 레지스터의 포인터로 본다"는 뜻이고, 앞의 `*`로 역참조해 그 레지스터 자체로 씁니다.

### 3) `volatile` — "매번 진짜로 읽어라"
컴파일러는 최적화를 위해 "값이 안 바뀐다"고 판단하면 메모리 읽기를 생략하고 레지스터(CPU 내부)에 캐싱합니다. 하지만 하드웨어 레지스터는 **하드웨어가 멋대로** 바꿉니다(센서 값 갱신, 플래그 셋 등). 캐싱하면 옛 값을 보게 되죠.

```c
while (!(UART_SR & TXE)) { }   /* TXE 가 1 이 될 때까지 대기 */
```

`UART_SR`이 `volatile`이 아니면, 컴파일러는 "루프 안에서 값이 안 변하네" 하고 한 번 읽은 값으로 무한 루프를 돌릴 수 있습니다. `volatile`을 붙이면 **매 반복마다 실제 주소에서 다시 읽어** 하드웨어 변화를 반영합니다. MMIO 포인터에 `volatile`은 사실상 필수입니다.

> 규칙: 하드웨어 레지스터, 인터럽트(ISR)와 메인이 공유하는 변수에는 `volatile`을 붙인다.

### 4) 구조체로 레지스터 블록 묶기 (CMSIS 스타일)
한 주변장치의 레지스터들은 보통 연속된 주소에 줄지어 있습니다. 그래서 CMSIS는 이들을 **구조체**로 묶고, 그 구조체 포인터를 시작 주소에 "얹습니다".

```c
typedef struct {
    volatile uint32_t MODER;   /* +0x00 */
    volatile uint32_t IDR;     /* +0x04 */
    volatile uint32_t ODR;     /* +0x08 */
} gpio_t;

#define GPIOC ((gpio_t *)0x40011000UL)
GPIOC->ODR |= (1u << 13);      /* 0x40011000 + 0x08 = ODR 에 접근 */
```

멤버 순서가 곧 오프셋(+0, +4, +8…)이라, `GPIOC->ODR`은 "시작 주소 + 8"을 정확히 가리킵니다. 13편 이후 `GPIOC->ODR`, `RCC->APB2ENR` 같은 표기가 모두 이 패턴입니다.

### 5) 포인터로 함수 밖 변수 바꾸기
함수에 값을 넘기면 복사본이 전달됩니다. 호출자의 원본을 바꾸려면 **주소**를 넘겨야 합니다.

```c
void clear_flag(uint8_t *reg) { *reg = 0; }   /* 호출자의 변수를 0 으로 */
```

## μVision 으로 보기 (이번 편은 PC로도 충분)
순수 로직이라 PC로 충분합니다. 실제 STM32에서는 `(*(volatile uint32_t*)0x4001100C)`를 Memory 창 주소에 입력해 값을 직접 보거나, Watch 창에서 `GPIOC->ODR`을 추가해 비트 변화를 관찰합니다(14편에서 실습).

## 예제로 보기

### 예제 1 — `main.c` : MMIO를 포인터로 흉내내기
변수 하나를 "가짜 레지스터"로 삼아, 그 주소를 가리키는 `volatile` 포인터로 비트를 set/clear/test 합니다. 실제 보드라면 포인터가 고정 주소를 가리킬 뿐 나머지는 동일합니다.

```c
#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/*
 * 04강 예제 1 — 메모리 맵드 I/O(MMIO) 개념을 PC에서 흉내낸다.
 * 실제 보드에서는 레지스터가 '고정된 주소'에 있고,
 *   #define GPIO_ODR (*(volatile uint32_t *)0x4001080CUL)
 * 처럼 그 주소를 가리키는 포인터를 역참조해 읽고 쓴다.
 * PC 에는 그런 주소가 없으므로 변수 하나를 '가짜 레지스터'로 삼아
 * 그 주소를 포인터로 다룬다. 동작 원리는 동일하다.
 */
int main(void)
{
    volatile uint32_t fake_reg = 0;       /* 실제론 고정 주소의 하드웨어 레지스터 */
    volatile uint32_t *reg = &fake_reg;   /* 그 주소를 가리키는 포인터 */

    *reg = 0x00000000u;                   /* 포인터 역참조로 레지스터에 쓰기 */
    printf("init      = 0x%08X\n", (unsigned)*reg);

    *reg |= BIT(3);                       /* set bit3 */
    printf("set bit3  = 0x%08X\n", (unsigned)*reg);

    *reg |= BIT(17);                      /* set bit17 */
    printf("set bit17 = 0x%08X\n", (unsigned)*reg);

    *reg &= ~BIT(3);                      /* clear bit3 */
    printf("clr bit3  = 0x%08X\n", (unsigned)*reg);

    /* 포인터로 레지스터를 읽어 특정 비트를 검사 */
    printf("bit17 is %s\n", ((*reg >> 17) & 1u) ? "ON" : "OFF");

    return 0;
}
```

### 예제 2 — `pc_test.c` : 구조체+포인터 레지스터 블록
주변장치를 구조체로 묶고 포인터로 접근하는 CMSIS 패턴을 확인합니다. 멤버가 +0/+4/+8 오프셋에 놓이는 것까지 검증합니다.

```c
#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/* 주변장치를 '레지스터 묶음' 구조체로 본다 (CMSIS 가 쓰는 방식).
 * 실제론 이 구조체를 고정 주소에 '얹어서' GPIO->ODR 처럼 접근한다.
 * 멤버 순서가 곧 레지스터의 오프셋(+0,+4,+8...)이 된다. */
typedef struct {
    volatile uint32_t MODER;   /* +0x00 모드 */
    volatile uint32_t IDR;     /* +0x04 입력 */
    volatile uint32_t ODR;     /* +0x08 출력 */
} gpio_t;

/*
 * 04강 예제 2 — 레지스터 블록을 구조체+포인터로 다루는 패턴을 검증한다.
 */
int main(void)
{
    gpio_t  storage;             /* PC: 일반 변수. 보드: 0x4001_0800 같은 고정 주소 */
    gpio_t *GPIO = &storage;     /* 그 주소에 구조체를 얹는다 */

    GPIO->ODR = 0;
    GPIO->ODR |= BIT(5);         /* PA5 출력 High 라고 가정 */
    printf("ODR after set PA5 = 0x%08X\n", (unsigned)GPIO->ODR);

    GPIO->ODR ^= BIT(5);         /* toggle */
    printf("ODR after toggle  = 0x%08X\n", (unsigned)GPIO->ODR);

    /* 구조체 멤버가 +0, +4, +8 오프셋에 놓이는지 확인 */
    printf("offset MODER=%u IDR=%u ODR=%u\n",
           (unsigned)((char *)&GPIO->MODER - (char *)GPIO),
           (unsigned)((char *)&GPIO->IDR   - (char *)GPIO),
           (unsigned)((char *)&GPIO->ODR   - (char *)GPIO));

    return 0;
}
```

## PC에서 검증하기

```
gcc main.c -o main && ./main
```

예상 출력(stdout):

```
init      = 0x00000000
set bit3  = 0x00000008
set bit17 = 0x00020008
clr bit3  = 0x00020000
bit17 is ON
```

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
ODR after set PA5 = 0x00000020
ODR after toggle  = 0x00000000
offset MODER=0 IDR=4 ODR=8
```

## 자주 하는 실수

### Q. `while (!(SR & TXE));`가 무한 루프에 빠져요.
A. `SR`이 `volatile`이 아니면 컴파일러가 "루프 안에서 안 변하니 한 번만 읽자"고 최적화해, 하드웨어가 비트를 바꿔도 보지 못합니다. 레지스터 포인터/매크로에 `volatile`을 붙여 매번 실제 주소에서 다시 읽게 하세요.

### Q. `p`와 `*p`가 헷갈려요.
A. `p`는 주소(어디), `*p`는 그 주소의 값(무엇). 레지스터 비트를 켜는 대상은 항상 `*p`(또는 `reg->FIELD`)입니다. `p |= ...`는 주소 자체를 망가뜨립니다.

### Q. `(uint32_t *)0x40011000`에 PC에서 접근하니 죽어요(segfault).
A. 당연합니다. 그 주소는 STM32에만 존재하는 주변장치 번지라, PC 메모리에는 없습니다. PC 연습에서는 이번 예제처럼 **변수의 주소**를 포인터에 담아 같은 패턴을 익히고, 실제 번지 접근은 14편 시뮬레이터/보드에서 합니다.

### Q. 주소를 정수로 쓸 때 `0x40011000`만 쓰면 되나요?
A. 32비트 주소 상수는 `0x40011000UL`처럼 부호 없는 long 접미사를 붙이는 게 안전합니다(이식성·경고 방지). 그리고 반드시 포인터 타입으로 캐스팅해서 씁니다: `(volatile uint32_t *)0x40011000UL`.

## 정리
- 임베디드 레지스터는 "고정 주소의 메모리"이고, 그 접근은 곧 포인터 역참조다.
- MMIO 관용구: `(*(volatile uint32_t *)ADDR)`로 특정 주소를 읽고 쓴다.
- `volatile`은 하드웨어가 바꾸는 값을 캐싱하지 말고 매번 다시 읽게 한다 — 레지스터엔 필수.
- 주변장치는 구조체+포인터로 묶으면 `GPIO->ODR`처럼 깔끔하게 접근된다(CMSIS 패턴).
- 함수 밖 변수를 바꾸려면 값이 아니라 주소(포인터)를 넘긴다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[05. 함수·헤더 분리·#define 매크로](../05_함수와_전처리기/README.md) — 코드를 함수와 헤더로 나누고, `#define`·매크로·헤더 가드로 펌웨어를 모듈답게 구성하는 법을 익힙니다.
