# 19. USART 시리얼 통신

지금까지는 LED와 핀으로만 결과를 봤지만, 이제 칩이 **사람·PC와 텍스트로 대화**합니다. UART(범용 비동기 직렬)는 가장 오래되고 가장 널리 쓰이는 통신으로, 디버그 메시지 출력, 센서 모듈 통신, 부트로더 등 어디에나 쓰입니다. STM32의 USART는 UART 기능을 포함하며, 두 가닥 선(TX/RX)만으로 양방향 통신을 합니다.

이번 편은 USART1을 9600bps로 설정해 인사 문자열을 보내고, 받은 문자를 그대로 되돌리는 **에코**를 구현합니다. μVision 시뮬레이터의 Serial 창에서 송수신 텍스트를 직접 확인할 수 있어, UART는 시뮬레이터로 검증하기 가장 좋은 주변장치입니다.

## 학습 목표
- UART의 비동기 통신 원리와 프레임(8N1)을 이해한다.
- USART1의 핀(PA9/PA10)과 클럭을 설정한다.
- `BRR`로 보율(baud rate)을 계산·설정한다.
- TXE/RXNE 플래그로 폴링 방식 송수신을 구현한다.
- μVision Serial 창에서 송수신을 확인한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (`View → Serial Windows → UART #1`). BRR 계산은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) UART 비동기 통신
UART는 클럭선 없이 양쪽이 **미리 약속한 속도(보율)** 로 데이터를 주고받습니다. 송신측 TX가 수신측 RX에 연결되고(교차), GND를 공유합니다. 속도가 안 맞으면 글자가 깨집니다.

### 2) 프레임 — 8N1
가장 흔한 설정은 **8N1**: 데이터 8비트, 패리티 없음(None), 정지비트 1개. 한 글자는 시작비트(1) + 데이터(8) + 정지비트(1) = 10비트로 전송됩니다.

### 3) USART1 핀과 클럭
| 신호 | 핀 | 모드 |
|------|----|----|
| TX | PA9 | AF 푸시풀(0xB) |
| RX | PA10 | 플로팅 입력(0x4) |

USART1은 **APB2** 버스라 `RCC_APB2ENR_USART1EN`으로 켭니다. 핀 재배치에 AFIO 클럭도 켜 둡니다.

### 4) 보율 설정 — BRR
```
BRR = fCK / baud
```
USART1의 fCK는 PCLK2(=72MHz)입니다. 9600bps면 `72000000 / 9600 = 7500`을 BRR에 씁니다.

### 5) 송수신 — 폴링
- **송신**: `SR`의 **TXE**(송신 버퍼 빔)를 기다렸다가 `DR`에 쓴다.
- **수신**: `SR`의 **RXNE**(수신 버퍼 참)를 확인하고 `DR`을 읽는다(읽으면 RXNE가 자동 클리어).

```c
while (!(USART1->SR & USART_SR_TXE)) { }   /* 보낼 준비 대기 */
USART1->DR = c;                            /* 한 글자 전송 */
```

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_usart`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).
8. `View → Serial Windows → UART #1`을 열어 둔다.

## 예제로 보기

### 예제 1 — `main.c` : 인사 + 에코
```c
#include "stm32f10x.h"

/*
 * 19강 예제 — USART1 송신 + 에코.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * USART1: TX=PA9, RX=PA10 (APB2 버스). 9600 8N1.
 * 시작 시 인사 문자열을 보내고, 이후 받은 문자를 그대로 돌려보낸다(에코).
 * 보율 분주값 BRR = fCK / baud, USART1 의 fCK 는 PCLK2(=72MHz).
 *   BRR = 72000000 / 9600 = 7500
 */

#define BAUD_BRR   7500u   /* 72MHz / 9600 */

static void usart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    /* PA9 = TX: AF 푸시풀(0xB), PA10 = RX: 플로팅 입력(0x4). 둘 다 CRH. */
    GPIOA->CRH &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->CRH |=  (0xBu << ((9u - 8u) * 4u));    /* PA9  AF 푸시풀 */
    GPIOA->CRH |=  (0x4u << ((10u - 8u) * 4u));   /* PA10 플로팅 입력 */

    USART1->BRR = BAUD_BRR;                        /* 9600 baud */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;  /* 송/수신 + 인에이블 */
}

static void usart1_send_char(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }      /* 송신 버퍼 빌 때까지 대기 */
    USART1->DR = (uint16_t)c;
}

static void usart1_send_str(const char *s)
{
    while (*s) {
        usart1_send_char(*s++);
    }
}

int main(void)
{
    usart1_init();
    usart1_send_str("Hello, STM32 UART!\r\n");

    while (1) {
        if (USART1->SR & USART_SR_RXNE) {          /* 수신 데이터 있음? */
            char c = (char)USART1->DR;             /* 읽으면 RXNE 자동 클리어 */
            usart1_send_char(c);                   /* 그대로 에코 */
        }
    }
}
```

### 예제 2 — `pc_test.c` : BRR 계산
```c
#include <stdio.h>
#include <stdint.h>

/*
 * 19강 PC 검산 — USART BRR(보율 분주값) 계산.
 * BRR = fCK / baud (USART1 은 PCLK2 = 72MHz).
 * 정수로 자르면 실제 보율은 fCK/BRR 이라 약간 오차가 생길 수 있다.
 * (72MHz 는 흔한 보율로 잘 나눠떨어져 오차가 0 이다.)
 */

static void report(uint32_t fck, uint32_t baud)
{
    uint32_t brr    = fck / baud;
    uint32_t actual = fck / brr;
    printf("baud %lu -> BRR=%lu (actual %lu)\n",
           (unsigned long)baud, (unsigned long)brr, (unsigned long)actual);
}

int main(void)
{
    uint32_t fck = 72000000u;
    report(fck, 9600u);
    report(fck, 19200u);
    report(fck, 38400u);
    report(fck, 57600u);
    report(fck, 115200u);
    return 0;
}
```

## 시뮬레이터로 확인하기
- 디버그 시작 후 `View → Serial Windows → UART #1`을 연다.
- 실행(F5)하면 UART #1 창에 `Hello, STM32 UART!`가 출력된다.
- UART #1 창에 글자를 입력하면(RX로 전달) 같은 글자가 에코되어 다시 나타난다.
- `USART1->BRR`가 `7500`(0x1D4C)인지, CR1의 UE/TE/RE가 1인지 레지스터로 확인.
- **예상 동작**: 인사 문자열 출력 후, 입력 문자가 그대로 되돌아온다.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
baud 9600 -> BRR=7500 (actual 9600)
baud 19200 -> BRR=3750 (actual 19200)
baud 38400 -> BRR=1875 (actual 38400)
baud 57600 -> BRR=1250 (actual 57600)
baud 115200 -> BRR=625 (actual 115200)
```

## 자주 하는 실수

### Q. 글자가 깨져서 나와요(□□□).
A. 보율 불일치가 가장 흔합니다. BRR 계산에 쓴 fCK가 실제 클럭과 맞는지(USART1은 PCLK2=72MHz), 터미널 보율이 같은지 확인하세요.

### Q. 첫 글자만 나오고 멈춰요.
A. TXE를 기다리지 않고 `DR`에 연속으로 쓰면 이전 글자를 덮어씁니다. 매 글자 전에 `while(!(SR & TXE))`로 대기하세요.

### Q. 에코가 안 돼요.
A. CR1에 RE(수신 인에이블)를 켰는지, RXNE를 확인하고 `DR`을 읽는지 보세요. `DR`을 읽어야 RXNE가 클리어되고 다음 수신이 가능합니다.

### Q. PA9/PA10을 일반 입출력으로 뒀어요.
A. TX(PA9)는 **AF 푸시풀(0xB)**, RX(PA10)는 입력이어야 USART 신호가 핀에 연결됩니다. 일반 출력으로 두면 USART가 핀을 못 씁니다.

## 정리
- UART는 보율을 맞춘 두 선(TX/RX)으로 비동기 통신한다(8N1 흔함).
- USART1은 PA9(TX)/PA10(RX), APB2 버스, BRR=fCK/baud.
- 송신은 TXE 대기 후 DR 쓰기, 수신은 RXNE 확인 후 DR 읽기.
- 시뮬레이터 UART #1 창으로 송수신 텍스트를 직접 본다.
- TX는 AF 푸시풀로 설정해야 한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[20. ADC 아날로그 입력](../20_ADC/README.md) — 디지털을 넘어 아날로그를 읽습니다. ADC로 전압을 숫자로 변환하고, 그 값을 UART로 출력해 봅니다.
