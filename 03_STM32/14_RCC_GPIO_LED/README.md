# 14. RCC·GPIO 레지스터로 LED

13편에서 잡은 토대(CMSIS·RCC·클럭 트리)를 처음으로 실전에 씁니다. STM32의 "Hello, World"인 **LED 점멸**을, HAL 없이 레지스터만으로 구현합니다. Blue Pill 계열 보드의 온보드 LED는 **PC13**에 액티브 로우로 달려 있어, 이 핀 하나를 출력으로 만들고 토글하면 깜빡임을 볼 수 있습니다.

핵심은 STM32F1 특유의 **GPIO 설정 방식**입니다. F4/L4 계열의 `MODER`와 달리, F1은 핀마다 4비트(MODE 2비트 + CNF 2비트)를 `CRL`(핀0~7)·`CRH`(핀8~15)에 씁니다. 이 4비트 조합을 이해하면 STM32F1의 모든 GPIO 설정이 같은 패턴으로 풀립니다.

## 학습 목표
- RCC로 GPIO 클럭을 켜고, F1의 CRL/CRH로 핀 모드를 설정한다.
- MODE/CNF 4비트 조합으로 "푸시풀 출력 2MHz"를 만든다.
- `ODR`로 핀을 토글하고, `BSRR/BRR`로 원자적 set/reset을 한다.
- 액티브 로우 LED(PC13)의 ON/OFF 논리를 이해한다.
- 시뮬레이터 레지스터/Logic Analyzer로 핀 토글을 확인한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (GPIOC 레지스터, Logic Analyzer PC13). 비트 조작은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) F1의 GPIO 설정 — CRL/CRH와 4비트
STM32F1은 핀 하나당 **4비트**로 모드를 정합니다. 핀 0~7은 `CRL`, 핀 8~15는 `CRH`에 있습니다. 핀 n의 4비트는 `(n % 8) * 4` 위치에 놓입니다.

| 비트 | 이름 | 의미 |
|------|------|------|
| [1:0] | MODE | 00=입력, 01=출력10MHz, 10=출력2MHz, 11=출력50MHz |
| [3:2] | CNF(출력) | 00=푸시풀, 01=오픈드레인, 10=AF푸시풀, 11=AF오픈드레인 |
| [3:2] | CNF(입력) | 00=아날로그, 01=플로팅, 10=풀업/풀다운, 11=예약 |

### 2) "푸시풀 출력 2MHz" = 0x2
LED 구동은 빠를 필요가 없으니 2MHz면 충분합니다. MODE=`10`(2MHz 출력), CNF=`00`(범용 푸시풀) → 4비트 `0010` = **0x2**.

```
PC13 → CRH, 위치 = (13 - 8) * 4 = 20번 비트부터 4비트
CRH 의 [23:20] 에 0x2 를 쓴다.
```

### 3) 설정의 정석 — 지우고(clear) 쓰기(set)
4비트만 바꾸려면 먼저 그 자리를 0으로 지운 뒤 새 값을 OR합니다. 다른 핀 설정을 건드리지 않기 위함입니다.

```c
GPIOC->CRH &= ~(0xFu << 20);   /* PC13 자리 4비트 클리어 */
GPIOC->CRH |=  (0x2u << 20);   /* 0010 설정 */
```

### 4) 핀 출력 — ODR vs BSRR/BRR
- `ODR`(Output Data Register): 포트 전체를 읽고 쓴다. 토글에 편하다(`ODR ^= (1<<13)`).
- `BSRR`(Bit Set/Reset): 하위 16비트=set, 상위 16비트=reset. **읽기 없이 원자적으로** 특정 핀만 1로.
- `BRR`(Bit Reset): 특정 핀만 0으로.

```c
GPIOC->BSRR = (1u << 13);          /* PC13 = 1 (액티브 로우면 LED OFF) */
GPIOC->BRR  = (1u << 13);          /* PC13 = 0 (LED ON) */
GPIOC->ODR ^= (1u << 13);          /* 토글 */
```

### 5) 액티브 로우 LED(PC13)
Blue Pill의 PC13 LED는 VCC—LED—저항—핀 구조라, **핀=0일 때 LED ON**입니다. 그래서 토글하면 절반 주기는 ON, 절반은 OFF가 됩니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_gpio_out`.
2. Device: **STMicroelectronics → STM32F1 Series → STM32F103 → STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup** 체크.
4. `main.c` 추가 후 아래 코드 복붙.
5. `Options → C/C++ → Define`에 `STM32F10X_MD`.
6. `Options → Debug → Use Simulator`, Dialog 파라미터 `-pSTM32F103C8`.
7. Rebuild(F7) → Start Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : PC13 LED 점멸
```c
#include "stm32f10x.h"

/*
 * 14강 예제 — 온보드 LED(PC13) 점멸.
 * 대상: STM32F103C8 (Blue Pill 계열), 컴파일러: Keil MDK-ARM.
 *
 * Blue Pill 보드는 PC13 에 LED 가 액티브 로우로 달려 있다(핀=0 이면 ON).
 * STM32F1 의 GPIO 설정은 CRL/CRH 레지스터로 핀마다 4비트(MODE+CNF)를 쓴다.
 */

#define LED_PIN   13u                 /* PC13 */

static void delay(volatile uint32_t count)
{
    while (count--) { }
}

int main(void)
{
    /* 1) GPIOC 클럭 켜기 (APB2 버스) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* 2) PC13 을 푸시풀 출력(2MHz)으로 설정.
     *    PC13 은 핀 8~15 영역이라 CRH 를 쓴다. 4비트 위치 = (13-8)*4 = 20.
     *    MODE=10(2MHz 출력), CNF=00(범용 푸시풀) → 4비트 값 0b0010 = 0x2. */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));   /* 해당 4비트 클리어 */
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));   /* 0010 설정 */

    while (1) {
        GPIOC->ODR ^= (1u << LED_PIN);   /* PC13 토글 */
        delay(200000u);
    }
}
```

### 예제 2 — `pc_test.c` : CRH 비트 조작 검산
```c
#include <stdio.h>
#include <stdint.h>

/*
 * 14강 PC 검산 — STM32F1 의 CRL/CRH 핀 설정 비트 조작을 흉내 낸다.
 * 핀 하나를 푸시풀 출력(2MHz, 4비트 값 0x2)으로 바꾸는 read-modify-write 를
 * 하드웨어 없이 계산한다. CRH 리셋값은 0x44444444(모든 핀 입력 플로팅).
 */

/* pin(8~15)을 mode4(4비트 설정)로 바꾼 새 CRH 값을 돌려준다. */
static uint32_t set_pin_config(uint32_t crh, unsigned int pin, uint32_t mode4)
{
    unsigned int shift = (pin - 8u) * 4u;
    crh &= ~(0xFu << shift);     /* 기존 4비트 지우고 */
    crh |=  (mode4 << shift);    /* 새 4비트 넣기 */
    return crh;
}

int main(void)
{
    uint32_t crh = 0x44444444u;     /* CRH 리셋값 */

    printf("reset CRH = 0x%08lX\n", (unsigned long)crh);
    crh = set_pin_config(crh, 13u, 0x2u);   /* PC13 = 푸시풀 출력 2MHz */
    printf("after PC13 output = 0x%08lX\n", (unsigned long)crh);

    return 0;
}
```

## 시뮬레이터로 확인하기
- `Peripherals → General Purpose I/O → GPIOC`(또는 Registers)에서 `CRH`가 `0x44244444`가 되는지 확인(PC13 자리만 0x2).
- Logic Analyzer에 `PORTC.13`을 Bit 모드로 추가하고 실행하면 PC13이 주기적으로 토글하는 사각파가 보인다.
- **예상 동작**: PC13이 일정 주기로 0↔1 토글(실보드라면 온보드 LED 깜빡임).

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
reset CRH = 0x44444444
after PC13 output = 0x44244444
```

## 자주 하는 실수

### Q. 분명히 설정했는데 핀이 안 움직여요.
A. GPIO 클럭(`RCC->APB2ENR |= RCC_APB2ENR_IOPCEN`)을 먼저 켰는지 확인하세요. 클럭이 없으면 CRH/ODR에 써도 반영되지 않습니다.

### Q. CRH인데 왜 PC13이 `(13-8)*4`인가요?
A. CRL은 핀0~7, CRH는 핀8~15를 담습니다. CRH 안에서 핀8이 0번째 4비트라, 핀13은 `(13-8)=5`번째 → 비트 20부터입니다.

### Q. 옆 핀 설정까지 같이 바뀌어요.
A. `=`로 통째로 덮어쓰면 다른 핀이 날아갑니다. 항상 `&= ~(0xF<<shift)`로 그 자리만 지우고 `|=`로 새 값을 넣으세요.

### Q. LED가 항상 켜져 있거나 꺼져 있어요.
A. 액티브 로우(PC13=0이 ON)임을 기억하세요. 또 `delay`가 없으면 너무 빨라 켜진 듯 보입니다. 토글 자체는 Logic Analyzer로 확인하는 게 정확합니다.

## 정리
- F1 GPIO는 핀당 4비트(MODE+CNF)를 CRL/CRH에 쓴다. 푸시풀 출력 2MHz = 0x2.
- 4비트만 바꿀 땐 "지우고(clear) 쓰기(set)" 패턴으로 다른 핀을 보존한다.
- 출력은 `ODR`(토글)·`BSRR/BRR`(원자적 set/reset)로 한다.
- PC13 LED는 액티브 로우 — 핀=0이 ON.
- 모든 동작의 전제는 RCC 클럭 인에이블이다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[15. GPIO 입력·풀업/풀다운](../15_GPIO_입력/README.md) — 이번엔 핀을 읽습니다. 입력 모드와 풀업/풀다운을 설정하고, 버튼 상태에 따라 LED를 제어합니다.
