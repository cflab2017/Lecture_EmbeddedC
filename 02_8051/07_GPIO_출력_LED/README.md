# 07. GPIO 출력 — LED 점멸

임베디드의 "Hello, World"는 LED 깜빡이기입니다. 핀 하나에 출력을 내보내 불을 켜고 끄는 일은 단순해 보이지만, **포트에 어떤 값을 써야 LED가 켜지는지**(배선 방식), **얼마나 기다려야 눈에 보이는지**(지연), **여러 LED로 패턴을 만드는 법**(비트 마스크)까지 GPIO 출력의 핵심이 다 들어 있습니다.

이번 편은 6편에서 익힌 포트 SFR 쓰기를 무한 루프 안에서 반복해, P1에 연결된 LED 8개로 **흐르는 불빛(running light)** 을 만듭니다. 많은 8051 보드가 LED를 **액티브 로우**(핀이 0일 때 ON)로 달기 때문에, "켜고 싶은 비트만 0"으로 만드는 감각이 중요합니다. 핀 파형은 μVision Logic Analyzer로 관찰합니다.

## 학습 목표
- GPIO 출력으로 LED를 제어하고, 액티브 로우/하이 배선의 차이를 설명한다.
- `while(1)` 안에서 포트 값을 바꿔 LED를 점멸시킨다.
- 소프트웨어 지연 루프의 동작과 한계(부정확)를 이해한다.
- 비트 마스크 `~(1u << pos)`로 흐르는 불빛 패턴을 만든다.
- μVision Logic Analyzer로 포트 핀의 토글을 관찰한다.

## 대상 환경
- 컴파일러: **Keil C51**.
- 디바이스: **AT89C52** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (Logic Analyzer / Port 1 창). 패턴 로직은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) LED 배선 — 액티브 로우 vs 액티브 하이
LED는 전류가 흘러야 켜집니다. 핀과 LED를 어떻게 연결하느냐에 따라 "켜는 값"이 달라집니다.

| 방식 | 배선 | LED ON 조건 |
|------|------|-------------|
| 액티브 하이 | 핀 → 저항 → LED → GND | 핀 = **1** |
| 액티브 로우 | VCC → LED → 저항 → 핀 | 핀 = **0** |

8051은 포트가 0을 출력할 때(싱크) 더 많은 전류를 흘릴 수 있어, 보드들이 흔히 **액티브 로우**로 답니다. 이 편 예제도 액티브 로우 가정입니다: **켜고 싶은 비트만 0**.

### 2) 한 LED만 켜기 — `~(1u << pos)`
액티브 로우에서 `pos`번 LED 하나만 켜려면, 그 비트만 0이고 나머지는 1인 값을 포트에 씁니다.

```
pos = 0 → ~(1<<0) = ~0x01 = 0xFE = 11111110   (LED0 만 ON)
pos = 3 → ~(1<<3) = ~0x08 = 0xF7 = 11110111   (LED3 만 ON)
```

`pos`를 0→7로 증가시키며 이 값을 반복해서 쓰면 불빛이 한 칸씩 흐릅니다.

### 3) 소프트웨어 지연
MCU는 너무 빨라서, 지연이 없으면 LED가 깜빡이는 게 아니라 흐릿하게 다 켜진 것처럼 보입니다. 가장 단순한 지연은 **빈 루프**입니다.

```c
static void delay(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) { }
}
```

이 방식은 간단하지만 **정확하지 않습니다**(클럭·최적화에 따라 시간이 달라짐). 정확한 시간은 9편의 하드웨어 타이머로 만듭니다. 지금은 시뮬레이터에서 관찰 가능한 정도면 충분합니다.

> 주의: 컴파일러가 빈 루프를 최적화로 제거할 수 있습니다. 실제 코드에서 정밀 지연이 필요하면 타이머를 쓰고, 굳이 빈 루프를 유지해야 하면 루프 변수를 `volatile`로 둡니다.

### 4) 여러 LED 패턴 — 마스크 조합
여러 비트를 OR로 묶으면 동시에 여러 LED를 켤 수 있습니다(액티브 로우면 그 묶음을 반전).

```c
unsigned int mask = (1u << pos) | (1u << (pos + 1));  /* 인접 2개 */
P1 = (unsigned char)~mask;                            /* 두 LED ON */
```

### 5) 포트 폭과 형 변환
`~(1u << pos)`는 `unsigned int`(8051에서 16비트) 결과라, 8비트 포트에 넣을 때 `(unsigned char)`로 캐스팅해 의도를 분명히 합니다. 상위 바이트는 어차피 버려지지만, 캐스팅으로 경고를 줄이고 "8비트만 쓴다"는 의도를 드러냅니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `c51_gpio_out`.
2. Device: **Atmel → AT89C52**.
3. STARTUP.A51 포함(Yes).
4. `main.c` 추가 후 아래 코드 복붙.
5. `Options for Target → Debug → Use Simulator`.
6. Rebuild(F7) → Start Debug(Ctrl+F5).
7. 핀 파형 보기: `Debug → Debug Settings`가 아니라, 디버그 중 `View → Analysis Windows → Logic Analyzer`를 열고 `Setup`에서 `P1.0`~`P1.7`을 Bit 모드로 추가한다. 또는 `Peripherals → I/O-Ports → Port 1`로 비트 상태를 직접 본다.

## 예제로 보기

### 예제 1 — `main.c` : 흐르는 불빛
한 LED만 켠 채 위치를 0→7로 옮기며 무한 반복합니다.

```c
#include <reg52.h>

/*
 * 07강 예제 1 — 흐르는 불빛(running light).
 * 대상: AT89C52, 컴파일러: Keil C51.
 *
 * P1 에 LED 8개가 연결돼 있다고 가정한다. 많은 8051 보드는 LED 를
 * '액티브 로우'로 단다(핀이 0 일 때 LED ON). 그래서 한 LED 만 켜려면
 * 그 비트만 0, 나머지는 1 인 값을 포트에 쓴다: ~(1 << pos).
 */

#define LED_PORT  P1

/* 소프트웨어 지연(정밀하지 않음). 시뮬레이터 관찰용으로 짧게 잡는다.
 * 정확한 시간 지연은 9편(타이머)에서 다룬다. */
static void delay(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) {
        /* 그냥 시간 보내기 */
    }
}

void main(void)
{
    unsigned char pos = 0;

    while (1) {
        /* 액티브 로우: pos 번 비트만 0 → 그 LED 하나만 ON */
        LED_PORT = (unsigned char)~(1u << pos);
        delay(20000);

        pos++;
        if (pos >= 8) {
            pos = 0;          /* 끝에 닿으면 처음 위치로 되돌아간다 */
        }
    }
}
```

### 예제 2 — `pc_test.c` : 패턴 시퀀스 검산
흐르는 불빛이 만들 포트 값(0xFE→0xFD→…→0x7F)을 PC에서 확인합니다.

```c
#include <stdio.h>

/*
 * 07강 예제 2 — 흐르는 불빛이 만들 포트 값 시퀀스를 PC 에서 검산한다.
 * 액티브 로우라 'pos 번 LED ON' = 그 비트만 0 = ~(1 << pos).
 */

static void print_bin8(unsigned char v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

int main(void)
{
    unsigned char pos;

    for (pos = 0; pos < 8; pos++) {
        unsigned char port = (unsigned char)~(1u << pos);
        printf("pos %u: ", (unsigned)pos);
        print_bin8(port);
        printf(" (0x%02X)\n", port);
    }

    return 0;
}
```

## 시뮬레이터로 확인하기
- Logic Analyzer에 P1.0~P1.7을 추가하고 실행(F5)하면, 각 핀이 차례로 Low(=LED ON)로 떨어지는 계단형 파형이 보인다.
- 또는 Port 1 창에서 체크 표시(비트 값)가 0xFE → 0xFD → 0xFB … 순으로 도는 것을 확인.
- **예상 동작**: LED가 0번→7번으로 한 칸씩 흐르고, 끝나면 다시 0번부터 반복.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
pos 0: 11111110 (0xFE)
pos 1: 11111101 (0xFD)
pos 2: 11111011 (0xFB)
pos 3: 11110111 (0xF7)
pos 4: 11101111 (0xEF)
pos 5: 11011111 (0xDF)
pos 6: 10111111 (0xBF)
pos 7: 01111111 (0x7F)
```

## 자주 하는 실수

### Q. 포트에 1을 썼는데 LED가 안 켜져요.
A. 보드가 액티브 로우면 1은 OFF입니다. **0**을 써야 켜집니다. 배선이 액티브 하이면 반대고요. 어느 쪽인지 모르면 둘 다 시도해 보거나 회로도를 확인하세요. 이 예제는 액티브 로우 가정입니다.

### Q. LED가 깜빡이지 않고 그냥 켜져 있어요.
A. 지연이 없거나 너무 짧아 사람 눈이 못 따라갑니다. `delay()` 값을 키우세요. 시뮬레이터에서는 실행 속도가 달라 보일 수 있으니, Logic Analyzer의 시간축으로 토글 자체를 확인하는 게 정확합니다.

### Q. 빈 지연 루프가 최적화로 사라져요.
A. 컴파일러가 "아무 일도 안 하는 루프"를 제거할 수 있습니다. 루프 변수를 `volatile`로 선언하거나(예: `volatile unsigned int i;`), 정확한 지연이 필요하면 타이머(9편)를 쓰세요.

### Q. `pos`가 8을 넘어 LED가 멈춰요.
A. `pos`가 8이 되면 `1<<8`이 포트 범위를 벗어납니다. `if (pos >= 8) pos = 0;`으로 0~7만 돌게 감싸세요(예제처럼).

## 정리
- LED 배선에 따라 켜는 값이 다르다 — 액티브 로우는 0이 ON.
- 한 LED만 켜기: 액티브 로우에서 `~(1u << pos)`.
- `while(1)` + 지연으로 점멸/패턴을 만든다. 소프트웨어 지연은 부정확하다.
- 여러 LED는 마스크를 OR로 묶어(필요시 반전) 제어한다.
- Logic Analyzer로 핀 파형을, Port 창으로 비트 값을 관찰한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[08. GPIO 입력 — 버튼/스위치 읽기](../08_GPIO_입력_버튼/README.md) — 이번엔 핀을 읽습니다. 8051 준쌍방향 포트로 버튼 입력을 받고, 입력에 따라 LED를 제어합니다.
