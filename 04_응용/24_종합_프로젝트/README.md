# 24. 종합 — UART 명령으로 LED·타이머 제어

드디어 마지막 편입니다. 지금까지 따로 배운 조각들 — **USART 통신(19), 인터럽트(18), 링버퍼(22), SysTick 타이머(16), GPIO LED(14), 명령 파싱·모듈 분리(22·23)** — 을 하나의 작은 펌웨어로 엮습니다. PC 터미널에서 `on`, `off`, `blink`, `help` 같은 명령을 타이핑하면, 보드가 그 명령을 해석해 LED와 타이머를 제어합니다.

이 캡스톤의 설계 핵심은 **역할 분리**입니다. 인터럽트는 받기만, 메인은 처리만, 파서는 해석만 합니다. 각 모듈(`ringbuffer`, `command`)은 독립적으로 테스트할 수 있고, 명령 파서는 하드웨어 없이 PC에서 검증합니다. 여기까지 오면 여러분은 "레지스터를 직접 다루면서도 구조가 깔끔한" 펌웨어를 짤 수 있게 된 것입니다.

## 학습 목표
- 여러 주변장치·모듈을 하나의 펌웨어로 통합한다.
- 인터럽트(수신)와 메인(처리)의 역할을 분리한다(생산자/소비자).
- 링버퍼로 UART 입력을 모으고 줄 단위로 조립한다.
- 명령을 파싱해 LED/타이머 동작에 연결한다.
- 파서를 모듈로 분리해 PC에서 단위 테스트한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision Serial(UART #1)로 명령 입력/응답. 파서는 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 전체 아키텍처
```
[PC 터미널] --UART--> RX 인터럽트 --push--> [링버퍼] --pop--> 메인 루프
                                                              │
                                              줄 조립 → command_parse() → handle_command()
                                                              │
                                              LED(PC13) 제어 / blink 모드 / UART 응답
                          SysTick(1ms) ──(blink 모드면 500ms 토글)──> LED
```

### 2) 모듈 구성
| 파일 | 책임 | 의존 |
|------|------|------|
| `ringbuffer.c/.h` | FIFO 버퍼 | 없음(순수) |
| `command.c/.h` | 줄 → 명령 코드 | 없음(순수) |
| `main.c` | 하드웨어 설정·통합 | CMSIS, 위 모듈 |
| `pc_test.c` | 파서 단위 테스트 | command(순수) |

순수 모듈(ringbuffer, command)은 칩에 의존하지 않아 그대로 PC에서 테스트됩니다.

### 3) 데이터 흐름 — 생산자/소비자
- **RX 인터럽트(생산자)**: 바이트를 `rb_push`로 넣기만 한다(짧게).
- **메인 루프(소비자)**: `rb_pop`으로 꺼내 줄을 조립하고, `\n`/`\r`에서 한 줄을 확정해 파싱·실행한다.

이 분리 덕분에 데이터가 쏟아져도 인터럽트는 빠르게 끝나고, 무거운 처리는 메인이 느긋하게 합니다.

### 4) 명령 프로토콜
| 명령 | 동작 | 응답 |
|------|------|------|
| `on` | LED 켜기, blink 해제 | `LED ON` |
| `off` | LED 끄기, blink 해제 | `LED OFF` |
| `blink` | 500ms 자동 점멸 모드 | `BLINK mode` |
| `help` | 명령 목록 | `cmds: ...` |

### 5) SysTick과 blink 모드
SysTick은 1ms마다 `g_ms`를 올리고, `blink_mode`가 켜져 있으면 500ms마다 LED를 토글합니다. `on`/`off`는 blink 모드를 끄고 LED를 고정합니다. `blink_mode`와 `g_ms`는 ISR·메인 공유라 `volatile`입니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `app_capstone`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. 소스 **4개**를 프로젝트에 추가: `main.c`, `ringbuffer.c`, `command.c` (+ 헤더는 같은 폴더). `pc_test.c`는 PC 검증용이라 Keil 프로젝트에는 넣지 않는다.
5. `C/C++ → Define`에 `STM32F10X_MD`. 헤더 경로(Include Paths)에 소스 폴더 추가.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5) → `View → Serial Windows → UART #1`.

## 예제로 보기

### 예제 1 — `command.h` / `command.c` : 명령 파서(순수 모듈)
```c
#ifndef COMMAND_H
#define COMMAND_H

/*
 * 명령 파서 — 한 줄 문자열을 명령 코드로 바꾼다.
 * 하드웨어 비의존 순수 로직이라 PC(gcc)에서 단위 테스트할 수 있다.
 */

typedef enum {
    CMD_NONE = 0,   /* 빈 줄 */
    CMD_ON,
    CMD_OFF,
    CMD_BLINK,
    CMD_HELP,
    CMD_UNKNOWN
} command_t;

command_t   command_parse(const char *line);
const char *command_name(command_t c);

#endif /* COMMAND_H */
```

```c
#include "command.h"

/* 표준 라이브러리 없이 동작하도록 간단한 문자열 비교를 직접 둔다. */
static int streq(const char *a, const char *b)
{
    while (*a != '\0' && (*a == *b)) {
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

command_t command_parse(const char *line)
{
    if (line[0] == '\0')      { return CMD_NONE;  }
    if (streq(line, "on"))    { return CMD_ON;    }
    if (streq(line, "off"))   { return CMD_OFF;   }
    if (streq(line, "blink")) { return CMD_BLINK; }
    if (streq(line, "help"))  { return CMD_HELP;  }
    return CMD_UNKNOWN;
}

const char *command_name(command_t c)
{
    switch (c) {
        case CMD_NONE:  return "NONE";
        case CMD_ON:    return "ON";
        case CMD_OFF:   return "OFF";
        case CMD_BLINK: return "BLINK";
        case CMD_HELP:  return "HELP";
        default:        return "UNKNOWN";
    }
}
```

### 예제 2 — `main.c` : 통합(요약)
전체 코드는 `src/app_capstone/main.c`에 있습니다. 핵심 흐름만 옮기면:

```c
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t b = (uint8_t)USART1->DR;   /* 읽으면 RXNE 클리어 */
        (void)rb_push(&rx_rb, b);          /* ISR 은 넣기만 — 짧게 */
    }
}

void SysTick_Handler(void)
{
    g_ms++;
    if (blink_mode && ((g_ms % 500u) == 0u)) {
        GPIOC->ODR ^= (1u << LED_PIN);     /* blink 모드: 500ms 토글 */
    }
}

int main(void)
{
    char line[LINE_MAX]; uint16_t idx = 0u; uint8_t c;

    rb_init(&rx_rb); led_init(); usart1_init();
    SysTick_Config(SystemCoreClock / 1000u);
    send_str("Capstone ready. Type: on/off/blink/help\r\n");

    while (1) {
        if (rb_pop(&rx_rb, &c)) {
            if (c == '\n' || c == '\r') {
                if (idx > 0u) { line[idx] = '\0'; handle_command(command_parse(line)); idx = 0u; }
            } else if (idx < (LINE_MAX - 1u)) {
                line[idx++] = (char)c;
            }
        }
    }
}
```

### 예제 3 — `pc_test.c` : 파서 단위 테스트
```c
#include <stdio.h>
#include "command.h"

/*
 * 24강 PC 검산 — 명령 파서 단위 테스트.
 * 입력 한 줄을 명령 코드로 바꾸는 로직만 하드웨어 없이 검증한다.
 * 빌드: gcc pc_test.c command.c -o pc_test
 */

int main(void)
{
    const char *lines[6] = { "on", "off", "blink", "help", "xyz", "" };
    int i;

    for (i = 0; i < 6; i++) {
        printf("\"%s\" -> %s\n", lines[i], command_name(command_parse(lines[i])));
    }
    return 0;
}
```

## 시뮬레이터로 확인하기
- 디버그 시작 후 `View → Serial Windows → UART #1`을 연다. 실행하면 `Capstone ready...`가 출력된다.
- UART #1 창에 `on` + Enter를 입력하면 `LED ON`이 응답되고 GPIOC ODR의 PC13이 0(켜짐)이 된다.
- `blink` 입력 → PC13이 500ms마다 토글(Logic Analyzer로 확인). `off` 입력 → 점멸 멈추고 OFF.
- `help` → 명령 목록 출력. 알 수 없는 명령 → `unknown command`.
- **예상 동작**: 입력 명령에 따라 LED 상태와 점멸 모드가 즉시 바뀌고, 응답 텍스트가 출력된다.

## PC에서 검증하기

```
gcc pc_test.c command.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
"on" -> ON
"off" -> OFF
"blink" -> BLINK
"help" -> HELP
"xyz" -> UNKNOWN
"" -> NONE
```

## 자주 하는 실수

### Q. 명령을 입력해도 반응이 없어요.
A. 줄 끝(`\n`/`\r`)이 와야 한 줄로 확정됩니다. 터미널이 Enter에 개행을 보내는지 확인하세요. 또 `RXNEIE`와 `NVIC_EnableIRQ(USART1_IRQn)`를 켰는지도요.

### Q. 빠르게 입력하면 글자가 사라져요.
A. 메인이 `rb_pop`을 충분히 자주 호출하는지, 링버퍼 크기가 충분한지 보세요. ISR에서 무거운 일을 하면 다음 바이트를 놓칩니다(ISR은 push만).

### Q. blink 모드인데 LED가 안 깜빡여요.
A. `SysTick_Config`를 호출했는지, `blink_mode`/`g_ms`가 `volatile`인지 확인하세요. `g_ms % 500` 토글 조건도 점검합니다.

### Q. 파서를 고쳤는데 보드에서 테스트하기 번거로워요.
A. 그래서 파서를 순수 모듈로 분리했습니다. `command.c`만 `pc_test.c`와 함께 gcc로 빌드해 로직을 먼저 검증하고, 보드엔 그 다음에 올리세요.

## 정리
- 캡스톤은 USART·인터럽트·링버퍼·SysTick·GPIO·파서를 하나로 엮는다.
- 인터럽트는 받아서 push만, 메인은 pop·조립·파싱·실행을 맡는다(분업).
- 순수 모듈(ringbuffer/command)은 PC에서 단위 테스트하고 보드에 올린다.
- 명령 프로토콜로 펌웨어를 외부에서 제어·확장할 수 있다.
- 레지스터 직접 제어와 깔끔한 구조는 양립한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 축하합니다 — 임베디드 C 입문 24편 완주! 🎉
1편의 `Hello, Embedded C!`에서 시작해, 8051과 STM32의 레지스터를 직접 다루고, 인터럽트·통신·타이머를 거쳐, 디바운싱·링버퍼·HAL 같은 펌웨어 패턴과 종합 프로젝트까지 왔습니다. 이제 여러분은:

- 데이터시트의 레지스터 표를 읽고 비트 단위로 제어할 수 있고,
- 폴링/인터럽트, 타이머/PWM, UART/ADC를 직접 설정할 수 있으며,
- 순수 로직을 PC에서 검증하고 HAL로 이식 가능한 구조를 설계할 수 있습니다.

다음 걸음은 실제 보드(Blue Pill, 8051 키트)에 이 예제들을 올려 보고, I2C/SPI 센서, RTOS, DMA 같은 주제로 확장하는 것입니다. 그동안 정말 수고하셨습니다!
