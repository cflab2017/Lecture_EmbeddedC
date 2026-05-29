# 18. NVIC 인터럽트와 EXTI

지금까지 버튼은 메인 루프가 계속 들여다보는 **폴링(polling)** 방식으로 읽었습니다. 하지만 CPU가 버튼만 쳐다보고 있으면 다른 일을 못 하고, 짧은 입력을 놓칠 수도 있습니다. **인터럽트(interrupt)** 는 이를 뒤집습니다. 평소엔 메인 코드가 자기 일을 하다가, 사건(핀 변화, 타이머 만료 등)이 생기면 CPU가 즉시 하던 일을 멈추고 **핸들러(ISR)** 로 점프했다가 돌아옵니다.

Cortex-M에서 인터럽트를 총괄하는 것이 **NVIC**(Nested Vectored Interrupt Controller)이고, 외부 핀의 변화를 인터럽트로 바꿔 주는 주변장치가 **EXTI**(External Interrupt/Event Controller)입니다. 이번 편은 PA0의 버튼 누름을 EXTI0 인터럽트로 받아 LED를 토글합니다.

## 학습 목표
- 폴링과 인터럽트의 차이와 장단점을 설명한다.
- NVIC의 역할과 `NVIC_EnableIRQ()`를 이해한다.
- EXTI로 핀의 에지(상승/하강)를 인터럽트로 만든다.
- AFIO->EXTICR로 핀을 EXTI 라인에 매핑한다.
- ISR을 작성하고 펜딩 플래그(PR)를 올바르게 클리어한다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (EXTI/NVIC 레지스터, PA0 강제 입력, PC13 관찰). ISR 로직은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) 폴링 vs 인터럽트
| 방식 | 동작 | 장점 | 단점 |
|------|------|------|------|
| 폴링 | 루프에서 계속 확인 | 단순, 흐름 명확 | CPU 낭비, 짧은 이벤트 놓침 |
| 인터럽트 | 사건 시 자동 호출 | 즉시 반응, CPU 여유 | 동기화 주의 필요 |

### 2) NVIC
NVIC는 Cortex-M 코어에 내장된 인터럽트 컨트롤러로, 각 인터럽트의 **활성화/우선순위/펜딩**을 관리합니다. CMSIS가 함수를 제공합니다.

```c
NVIC_EnableIRQ(EXTI0_IRQn);          /* 인터럽트 활성화 */
NVIC_SetPriority(EXTI0_IRQn, 2);     /* 우선순위(숫자 작을수록 높음) */
```

### 3) EXTI — 핀 변화를 인터럽트로
EXTI는 16개 라인(EXTI0~15)을 갖고, 각 라인의 **상승/하강 에지**를 감지해 인터럽트를 냅니다. 주요 레지스터:

| 레지스터 | 역할 |
|----------|------|
| `IMR` | 인터럽트 마스크(해당 라인 인터럽트 허용) |
| `RTSR` | 상승 에지 트리거 |
| `FTSR` | 하강 에지 트리거 |
| `PR` | 펜딩(발생) 플래그 — **1을 써서 클리어** |

### 4) AFIO->EXTICR — 핀→라인 매핑
EXTI 라인 n은 여러 포트의 핀 n 중 하나에 연결할 수 있습니다. 어느 포트인지는 `AFIO->EXTICR[]`로 고릅니다. 라인 n의 설정은 `EXTICR[n/4]`의 `(n%4)*4` 위치 4비트입니다(0=PA, 1=PB, 2=PC…).

```c
AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;   /* EXTI0 ← PA0 (필드 0) */
```

> EXTI 매핑에는 AFIO 클럭(`RCC_APB2ENR_AFIOEN`)이 필요합니다.

### 5) ISR과 펜딩 클리어
인터럽트 핸들러 이름은 startup 벡터 테이블과 일치해야 합니다(예: `EXTI0_IRQHandler`). 핸들러 안에서 **반드시 PR을 클리어**해야 합니다. 안 그러면 핸들러를 나가자마자 다시 같은 인터럽트가 걸려 무한 반복합니다.

```c
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR = EXTI_PR_PR0;        /* 1 을 써야 클리어된다 */
        /* ... 처리 ... */
    }
}
```

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_exti`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : EXTI0로 LED 토글
```c
#include "stm32f10x.h"

/*
 * 18강 예제 — EXTI0(PA0) 외부 인터럽트로 LED 토글.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 버튼을 PA0 와 VCC 사이에 달고 내부 풀다운을 켠다(평소 0, 누르면 1).
 * 누르는 순간의 '상승 에지'를 EXTI 라인0 이 잡아 인터럽트를 건다.
 * 메인 루프는 아무 일도 안 하고, 토글은 인터럽트 핸들러가 처리한다.
 */

#define BTN_PIN   0u     /* PA0 / EXTI0 */
#define LED_PIN   13u    /* PC13 */

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0) {        /* 라인0 펜딩? */
        EXTI->PR = EXTI_PR_PR0;          /* 1 을 써서 펜딩 클리어 */
        GPIOC->ODR ^= (1u << LED_PIN);   /* LED 토글 */
    }
}

int main(void)
{
    /* 클럭: GPIOA, GPIOC, 그리고 EXTI 매핑에 필요한 AFIO */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PA0: 입력 + 풀다운 (버튼은 PA0~VCC, 누르면 상승 에지) */
    GPIOA->CRL &= ~(0xFu << (BTN_PIN * 4u));
    GPIOA->CRL |=  (0x8u << (BTN_PIN * 4u));
    GPIOA->ODR &= ~(1u << BTN_PIN);                   /* ODR=0 → 풀다운 */

    /* PC13: 푸시풀 출력 2MHz */
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));

    /* EXTI0 라인을 PA0 에 연결 (EXTICR1 의 EXTI0 필드 = 0 → 포트 A) */
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;

    /* 라인0: 상승 에지 트리거 + 인터럽트 마스크 해제 */
    EXTI->RTSR |= EXTI_RTSR_TR0;
    EXTI->IMR  |= EXTI_IMR_MR0;

    /* NVIC 에서 EXTI0 인터럽트 활성화 */
    NVIC_EnableIRQ(EXTI0_IRQn);

    while (1) {
        /* 메인은 한가하다 — 버튼 처리는 인터럽트가 한다 */
    }
}
```

### 예제 2 — `pc_test.c` : ISR 처리 로직 검산
```c
#include <stdio.h>
#include <stdint.h>

/*
 * 18강 PC 검산 — EXTI 인터럽트 처리 로직 흉내.
 * 에지가 발생할 때마다 ISR 이 LED 를 토글하고 카운터를 늘린다.
 * (실제 하드웨어에서는 펜딩 플래그를 클리어해야 다음 인터럽트가 걸린다.)
 */

int main(void)
{
    int events[8] = { 1, 0, 1, 1, 0, 1, 0, 1 };  /* 1 = 에지 발생 */
    uint32_t count = 0;
    int led = 0;
    int i;

    for (i = 0; i < 8; i++) {
        if (events[i]) {            /* 인터럽트 발생 */
            count++;
            led = !led;             /* ISR 동작: LED 토글 */
        }
        printf("t=%d event=%d -> count=%lu LED=%s\n",
               i, events[i], (unsigned long)count, led ? "ON" : "OFF");
    }
    return 0;
}
```

## 시뮬레이터로 확인하기
- 디버그 중 Command 창에서 `PORTA |= 0x0001`(PA0=1, 상승 에지)을 실행하면 EXTI0 인터럽트가 걸려 `EXTI0_IRQHandler`로 진입한다(중단점으로 확인).
- 핸들러가 `EXTI->PR`을 클리어하고 PC13을 토글한다. `PORTA &= ~0x0001` 후 다시 `|= 0x0001`로 또 토글.
- NVIC 창에서 EXTI0이 Enabled인지, EXTI 레지스터에서 IMR/RTSR 비트0이 1인지 확인.
- **예상 동작**: PA0 상승 에지마다 PC13이 토글(폴링 없이).

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
t=0 event=1 -> count=1 LED=ON
t=1 event=0 -> count=1 LED=ON
t=2 event=1 -> count=2 LED=OFF
t=3 event=1 -> count=3 LED=ON
t=4 event=0 -> count=3 LED=ON
t=5 event=1 -> count=4 LED=OFF
t=6 event=0 -> count=4 LED=OFF
t=7 event=1 -> count=5 LED=ON
```

## 자주 하는 실수

### Q. 인터럽트가 한 번 걸리고 영원히 핸들러에서 못 나와요.
A. `EXTI->PR`을 클리어하지 않았습니다. PR은 **1을 써야** 클리어됩니다(`EXTI->PR = EXTI_PR_PR0;`). 클리어를 빠뜨리면 핸들러를 나가자마자 재진입합니다.

### Q. 핸들러가 아예 호출되지 않아요.
A. 세 가지를 확인하세요. ① `RCC_APB2ENR_AFIOEN`(AFIO 클럭), ② `EXTI->IMR` 마스크 해제, ③ `NVIC_EnableIRQ`. 그리고 핸들러 이름이 `EXTI0_IRQHandler`로 정확한지.

### Q. PA0인데 EXTICR를 어떻게 설정하죠?
A. 라인 n은 `EXTICR[n/4]`의 `(n%4)*4` 위치 4비트입니다. EXTI0은 `EXTICR[0]`의 하위 4비트, 값 0이면 PA0입니다.

### Q. ISR 안에서 시간이 오래 걸리는 일을 해도 되나요?
A. 안 됩니다. ISR은 **짧게** 유지하세요. 긴 작업은 플래그만 세우고 메인 루프에서 처리하는 패턴이 정석입니다(22편 링버퍼에서 활용).

## 정리
- 인터럽트는 사건 발생 시 자동 호출돼 CPU를 효율적으로 쓴다.
- NVIC가 인터럽트를 총괄하고, `NVIC_EnableIRQ`로 켠다.
- EXTI는 핀 에지를 인터럽트로 바꾸며 IMR/RTSR/FTSR/PR로 제어한다.
- 핀→라인 매핑은 AFIO->EXTICR(AFIO 클럭 필요)로 한다.
- ISR에서 펜딩(PR)을 반드시 클리어하고, 핸들러는 짧게 유지한다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[19. USART 시리얼 통신](../19_USART/README.md) — 칩과 PC가 텍스트로 대화합니다. USART로 문자를 보내고 받아, μVision Serial 창에서 출력을 확인합니다.
