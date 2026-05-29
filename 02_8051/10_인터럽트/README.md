# 10. 외부·타이머 인터럽트

9편의 폴링 지연은 정확하지만, 기다리는 동안 CPU가 묶입니다. 버튼을 폴링하면 계속 들여다봐야 하고요. **인터럽트(interrupt)** 는 이 문제를 뒤집습니다. 평소엔 메인 코드가 제 일을 하다가, 특정 사건(버튼 눌림, 타이머 오버플로)이 생기면 CPU가 **잠깐 끼어들어** 정해진 함수(ISR)를 실행하고 원래 자리로 돌아옵니다. "사건이 생길 때만 반응"하는 이 방식이 임베디드의 핵심 패러다임입니다.

이번 편은 8051의 **외부 인터럽트(INT0)** 와 **타이머 인터럽트**를 다룹니다. `IE` 레지스터로 인터럽트를 켜고, Keil C51의 `interrupt N` 문법으로 ISR을 작성하며, ISR과 메인이 공유하는 변수에는 왜 `volatile`이 필요한지 익힙니다.

## 학습 목표
- 인터럽트의 개념(ISR, 벡터, 비동기 사건)을 폴링과 비교해 설명한다.
- `IE` 레지스터(EA/EX0/ET0 등)로 인터럽트를 허용한다.
- Keil C51의 `void isr(void) interrupt N` 문법으로 ISR을 작성한다.
- 외부 인터럽트(INT0)와 타이머 인터럽트를 각각 설정한다.
- ISR과 메인이 공유하는 변수에 `volatile`을 붙이는 이유를 안다.

## 대상 환경
- 컴파일러: **Keil C51**.
- 디바이스: **AT89C52** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (Port 3로 INT0 트리거, Port 1로 LED 관찰). ISR 로직은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 인터럽트란
사건이 발생하면 하드웨어가 자동으로 현재 코드를 멈추고, 미리 정해진 주소(**인터럽트 벡터**)의 함수(**ISR**, Interrupt Service Routine)로 점프합니다. ISR이 끝나면 멈췄던 자리로 돌아옵니다.

| | 폴링 | 인터럽트 |
|---|------|----------|
| 사건 감지 | 계속 확인 | 발생 시 자동 호출 |
| CPU 효율 | 낮음(대기 낭비) | 높음(평소 자유) |
| 반응 지연 | 루프 주기에 의존 | 즉각적 |

### 2) 8051 인터럽트 소스와 번호
Keil C51은 인터럽트마다 번호를 부여합니다(ISR 선언에 사용).

| 번호 | 소스 | 플래그 |
|------|------|--------|
| 0 | 외부 인터럽트 0 (INT0, P3.2) | IE0 |
| 1 | 타이머 0 오버플로 | TF0 |
| 2 | 외부 인터럽트 1 (INT1, P3.3) | IE1 |
| 3 | 타이머 1 오버플로 | TF1 |
| 4 | 시리얼(UART) | RI/TI |

### 3) IE — 인터럽트 인에이블 레지스터
| 비트 | 이름 | 역할 |
|------|------|------|
| EA | 전역 허용 | 0이면 모든 인터럽트 차단(마스터 스위치) |
| ES | 시리얼 | |
| ET1 / ET0 | 타이머1 / 타이머0 | |
| EX1 / EX0 | 외부1 / 외부0 | |

**규칙: 개별 인터럽트를 켜고(예: `EX0 = 1`), 마지막에 전역 `EA = 1`** 을 켭니다. `EA`가 0이면 무엇을 켜도 동작하지 않습니다.

### 4) 외부 인터럽트의 트리거 모드 — 레벨 vs 에지
`IT0`(TCON의 비트)으로 INT0 트리거 방식을 고릅니다.

- `IT0 = 0`: **레벨** 트리거(핀이 Low인 동안 계속). 버튼을 누르고 있으면 ISR이 반복될 수 있음.
- `IT0 = 1`: **하강 에지** 트리거(High→Low 순간 한 번). 버튼 "누르는 순간"에 적합.

### 5) ISR 문법과 `volatile`
Keil C51의 ISR은 반환·인자가 없고 `interrupt N`을 붙입니다.

```c
void int0_isr(void) interrupt 0   /* 외부 인터럽트 0 */
{
    /* 짧고 빠르게! printf 같은 무거운 작업은 피한다 */
}
```

ISR과 메인이 **같은 변수**를 쓰면, 컴파일러는 메인 입장에서 "이 변수는 안 바뀐다"고 오해해 캐싱할 수 있습니다. ISR이 비동기로 바꾸므로, 공유 변수에는 `volatile`을 붙여 **매번 메모리에서 다시 읽게** 합니다.

```c
volatile unsigned char press_count;   /* ISR ↔ main 공유 */
```

> ISR은 짧게 유지하세요. 무거운 작업(지연, UART 송신 등)은 플래그만 세우고 메인에서 처리합니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `c51_interrupt`.
2. Device: **Atmel → AT89C52**. STARTUP.A51 포함(Yes).
3. `main.c` 추가 후 아래 코드 복붙.
4. `Options for Target → Debug → Use Simulator`.
5. Rebuild(F7) → Start Debug(Ctrl+F5).
6. `Peripherals → I/O-Ports → Port 3`을 열어 P3.2를 1→0으로 토글(외부 인터럽트 트리거), `Port 1`에서 LED 관찰. `Peripherals → Interrupt`에서 인터럽트 상태도 볼 수 있다.

## 예제로 보기

### 예제 1 — `main.c` : 외부 인터럽트로 버튼 처리
INT0(P3.2)이 하강 에지가 되면 ISR이 LED를 토글합니다. 메인 루프는 비어 있습니다.

```c
#include <reg52.h>

/*
 * 10강 예제 1 — 외부 인터럽트(INT0)로 버튼 처리.
 * 대상: AT89C52, 컴파일러: Keil C51.
 *
 * 폴링(8편)은 main 이 계속 버튼을 들여다봐야 한다. 인터럽트는 버튼이
 * 눌리는 '순간'에만 CPU 가 잠깐 끼어들어 ISR 을 실행하고, 평소 main 은
 * 자유롭다. INT0 핀(P3.2)이 하강 에지가 되면 int0_isr() 가 호출된다.
 */

sbit LED = P1 ^ 0;                       /* 액티브 로우 LED */

/* ISR 와 main 이 함께 쓰는 변수는 volatile (컴파일러 캐싱 방지). */
volatile unsigned char press_count = 0;

/* 외부 인터럽트 0 서비스 루틴: interrupt 0 (Keil C51 문법). */
void int0_isr(void) interrupt 0
{
    press_count++;          /* 눌린 횟수 누적 */
    LED = (bit)!LED;        /* LED 토글 */
}

void main(void)
{
    IT0 = 1;     /* INT0 를 '하강 에지'에서 트리거(레벨 아님) */
    EX0 = 1;     /* 외부 인터럽트 0 허용 */
    EA  = 1;     /* 전역 인터럽트 허용 (마스터 스위치) */

    while (1) {
        /* 메인은 자유. 버튼 처리는 인터럽트가 알아서 한다.
         * 필요하면 여기서 press_count 로 다른 동작을 할 수 있다. */
    }
}
```

### 예제 2 — `pc_test.c` : ISR 로직 검증
인터럽트가 N번 발생했을 때의 카운트와 LED 상태를 PC에서 확인합니다.

```c
#include <stdio.h>

/*
 * 10강 예제 2 — ISR 가 하는 일(카운트 증가 + LED 토글)을 PC 에서 검증.
 * 실제로는 인터럽트가 비동기로 발생하지만, 여기서는 '인터럽트 N번 발생'을
 * 순서대로 적용해 최종 상태(press_count, LED)를 확인한다.
 * 액티브 로우라 led 변수 0 = ON, 1 = OFF.
 */
int main(void)
{
    unsigned char led = 1;       /* 시작: OFF (1=OFF) */
    unsigned int count = 0;
    int events = 5;              /* 인터럽트 5번 발생 가정 */
    int e;

    for (e = 1; e <= events; e++) {
        count++;
        led = (unsigned char)!led;
        printf("IRQ %d: count=%u led=%s\n",
               e, count, led ? "OFF" : "ON");
    }

    return 0;
}
```

## 시뮬레이터로 확인하기
- 실행(F5) 후 `Port 3` 창에서 P3.2를 1→0으로 바꾼다(버튼 누름 흉내).
- **예상 동작**: P3.2가 하강할 때마다 `int0_isr`이 호출되어 P1.0 LED가 토글된다. 누를 때마다 ON↔OFF가 바뀐다. `press_count`를 Watch에 추가하면 누를 때마다 1씩 증가.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
IRQ 1: count=1 led=ON
IRQ 2: count=2 led=OFF
IRQ 3: count=3 led=ON
IRQ 4: count=4 led=OFF
IRQ 5: count=5 led=ON
```

## 자주 하는 실수

### Q. ISR이 전혀 호출되지 않아요.
A. 십중팔구 `EA = 1`(전역 허용)을 안 켰습니다. 개별 인에이블(`EX0`/`ET0` 등)만으로는 부족하고, 마스터 스위치 `EA`가 켜져야 합니다. 또 ISR의 `interrupt N` 번호가 소스와 맞는지 확인하세요(외부0=0, 타이머0=1).

### Q. ISR에서 바꾼 변수가 메인에서 안 바뀐 것처럼 보여요.
A. 공유 변수에 `volatile`을 빠뜨렸을 가능성이 큽니다. 컴파일러가 메인에서 그 변수를 캐싱하면 ISR의 변경을 못 봅니다. `volatile`을 붙이세요.

### Q. 버튼을 한 번 눌렀는데 ISR이 여러 번 불려요.
A. `IT0 = 0`(레벨 트리거)이면 누르고 있는 동안 계속 트리거됩니다. 누르는 순간 한 번만 원하면 `IT0 = 1`(에지 트리거)로 바꾸세요. 접점 떨림(채터링)이 원인이면 디바운싱(21편)이 필요합니다.

### Q. ISR 안에서 긴 작업을 했더니 시스템이 버벅여요.
A. ISR은 짧아야 합니다. ISR 동안 다른 인터럽트가 지연되고 메인이 멈춥니다. 무거운 일은 ISR에서 `volatile` 플래그만 세우고, 메인 루프에서 그 플래그를 보고 처리하세요.

## 정리
- 인터럽트는 사건 발생 시에만 ISR을 실행해, 폴링의 대기 낭비를 없앤다.
- `IE`의 개별 비트(EX0/ET0…)를 켜고 마지막에 `EA = 1`로 전역 허용한다.
- Keil C51 ISR은 `void isr(void) interrupt N` 형태(외부0=0, 타이머0=1…).
- 외부 인터럽트는 `IT0`로 레벨/에지 트리거를 고른다.
- ISR↔메인 공유 변수에는 `volatile`을 붙이고, ISR은 짧게 유지한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[11. UART 시리얼 통신](../11_UART/README.md) — 칩과 PC가 텍스트로 대화합니다. UART로 문자를 보내고 받아, μVision Serial 창에서 출력을 확인합니다.
