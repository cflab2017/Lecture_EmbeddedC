# 22. 링버퍼와 인터럽트 기반 UART 수신

18편에서 인터럽트는 "짧게" 유지하라고 배웠습니다. 그런데 UART로 데이터가 쏟아져 들어오면, 수신 인터럽트(RXNE)가 매 바이트마다 걸립니다. 이 바이트들을 ISR 안에서 다 처리하려 들면 ISR이 길어지고, 메인이 느리면 다음 바이트가 도착해 **이전 바이트를 덮어써 유실**됩니다. 해법은 ISR은 받은 바이트를 **버퍼에 넣기만** 하고, 실제 처리는 한가한 메인 루프가 맡는 분업입니다.

이 분업의 핵심 자료구조가 **링버퍼(원형 버퍼)** 입니다. 생산자(ISR)와 소비자(메인)가 서로 다른 인덱스만 건드리게 설계하면, 인터럽트를 끄지 않고도 안전하게 데이터를 주고받을 수 있습니다. 이번 편은 링버퍼를 하드웨어 비의존 모듈로 구현하고 PC에서 검증한 뒤, 실제 UART RX 인터럽트와 어떻게 결합하는지 봅니다.

## 학습 목표
- 인터럽트 수신에서 데이터 유실이 왜 생기는지 설명한다.
- 링버퍼의 head/tail FIFO 구조를 이해한다.
- 빈/가득 판정과 2의 거듭제곱 마스킹을 구현한다.
- 단일 생산자/소비자(SPSC) 환경의 안전성과 `volatile`을 이해한다.
- 링버퍼를 UART RX 인터럽트와 결합하는 패턴을 안다.

## 대상 환경
- 컴파일러: **PC + gcc** 또는 μVision(순수 로직).
- 디바이스: 없음(하드웨어 비의존). 실제 보드에선 생산자가 USART RX ISR이다.
- 검증: PC `gcc`(`pc_test.c`)로 예상 출력 확인.

## 핵심 개념

### 1) 인터럽트 수신의 딜레마
RX 인터럽트는 바이트마다 발생합니다. ISR이 길거나 메인이 그 바이트를 제때 안 가져가면, 다음 바이트가 수신 레지스터를 덮어써 데이터가 사라집니다. **"빨리 받아서 어딘가 쌓아두고, 처리는 나중에"** 가 필요합니다.

### 2) 링버퍼 구조
크기가 고정된 배열을 원형으로 씁니다. **head**(쓰기 위치)와 **tail**(읽기 위치)이 끝에 도달하면 다시 0으로 감싸 돕니다. 먼저 들어간 것이 먼저 나오는 **FIFO**입니다.

```
[ . . A B C . . . ]
      ^tail   ^head      pop 은 tail 에서, push 는 head 에서
```

### 3) 빈/가득 판정
- **빈 상태**: `head == tail`.
- **가득 상태**: `((head + 1) & MASK) == tail`.

가득 판정을 위해 슬롯 하나를 비워 두므로, 크기 `N`이면 실제 용량은 `N-1`입니다. 크기가 2의 거듭제곱이면 `% N` 대신 `& (N-1)`로 빠르게 감쌀 수 있습니다.

```c
#define RB_SIZE 8u
#define RB_MASK (RB_SIZE - 1u)
next = (head + 1) & RB_MASK;   /* 8 → 0 으로 자동 wrap */
```

### 4) SPSC 안전성과 volatile
생산자(ISR)는 **head만**, 소비자(메인)는 **tail만** 씁니다. 서로 다른 변수를 갱신하므로, 단일 생산자/단일 소비자(SPSC)에서는 락이나 인터럽트 차단 없이도 안전합니다. 단, head/tail은 ISR과 메인이 공유하므로 **`volatile`** 로 선언해 컴파일러 캐싱을 막아야 합니다.

### 5) UART RX 인터럽트와 결합(개념)
```c
/* USART1 수신 인터럽트: 받은 바이트를 버퍼에 넣기만 한다 (짧게!) */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t b = (uint8_t)USART1->DR;   /* 읽으면 RXNE 클리어 */
        rb_push(&rx_rb, b);                /* 생산자: head 만 건드림 */
    }
}

int main(void)
{
    /* ... USART1 + RXNEIE 인터럽트 설정 ... */
    uint8_t c;
    while (1) {
        if (rb_pop(&rx_rb, &c)) {          /* 소비자: tail 만 건드림 */
            /* 받은 바이트 c 를 느긋하게 처리 */
        }
    }
}
```

> `USART1->CR1`에 `USART_CR1_RXNEIE`(수신 인터럽트 인에이블)를 켜고 `NVIC_EnableIRQ(USART1_IRQn)`를 호출하면 RX 인터럽트가 동작합니다. 처리는 메인이 합니다.

## 예제로 보기

### 예제 1 — `ringbuffer.h` / `ringbuffer.c` : 링버퍼 모듈
```c
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

/*
 * 원형 버퍼(링버퍼) — 인터럽트 수신과 메인 처리를 잇는 FIFO.
 * 생산자(ISR)는 push 로 head 만, 소비자(메인)는 pop 으로 tail 만 건드린다.
 * 그래서 인터럽트를 끄지 않고도 단일 생산자/단일 소비자(SPSC)에서 안전하다.
 *
 * 크기는 2의 거듭제곱이어야 (& MASK) 로 인덱스를 감쌀 수 있다.
 * '한 칸 비우기' 방식이라 실제 용량은 RB_SIZE - 1 이다.
 */

#define RB_SIZE  8u            /* 2의 거듭제곱 */

typedef struct {
    uint8_t  buf[RB_SIZE];
    volatile uint16_t head;    /* 다음에 쓸 위치 (생산자) */
    volatile uint16_t tail;    /* 다음에 읽을 위치 (소비자) */
} ringbuffer_t;

void     rb_init(ringbuffer_t *rb);
uint8_t  rb_is_empty(const ringbuffer_t *rb);
uint8_t  rb_is_full(const ringbuffer_t *rb);
uint8_t  rb_push(ringbuffer_t *rb, uint8_t data);   /* 성공 1, 가득참 0 */
uint8_t  rb_pop(ringbuffer_t *rb, uint8_t *out);     /* 성공 1, 비었음 0 */
uint16_t rb_count(const ringbuffer_t *rb);

#endif /* RINGBUFFER_H */
```

```c
#include "ringbuffer.h"

#define RB_MASK  (RB_SIZE - 1u)

void rb_init(ringbuffer_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
}

uint8_t rb_is_empty(const ringbuffer_t *rb)
{
    return (rb->head == rb->tail) ? 1u : 0u;
}

uint8_t rb_is_full(const ringbuffer_t *rb)
{
    return (((rb->head + 1u) & RB_MASK) == rb->tail) ? 1u : 0u;
}

uint8_t rb_push(ringbuffer_t *rb, uint8_t data)
{
    uint16_t next = (uint16_t)((rb->head + 1u) & RB_MASK);
    if (next == rb->tail) {
        return 0u;                  /* 가득 참 — 데이터 유실 방지 위해 거절 */
    }
    rb->buf[rb->head] = data;
    rb->head = next;
    return 1u;
}

uint8_t rb_pop(ringbuffer_t *rb, uint8_t *out)
{
    if (rb->head == rb->tail) {
        return 0u;                  /* 비었음 */
    }
    *out = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1u) & RB_MASK);
    return 1u;
}

uint16_t rb_count(const ringbuffer_t *rb)
{
    return (uint16_t)((rb->head - rb->tail) & RB_MASK);
}
```

### 예제 2 — `pc_test.c` : FIFO·가득 참 검증
```c
#include <stdio.h>
#include <stdint.h>
#include "ringbuffer.h"

/*
 * 22강 PC 검산 — 링버퍼 FIFO 동작.
 * 넣기/꺼내기, 가득 참(용량 RB_SIZE-1=7) 거절, FIFO 순서를 확인한다.
 * 빌드: gcc pc_test.c ringbuffer.c -o pc_test
 */

int main(void)
{
    ringbuffer_t rb;
    uint8_t c;
    char x;

    rb_init(&rb);

    /* 1) 3개 넣기 */
    rb_push(&rb, (uint8_t)'A');
    rb_push(&rb, (uint8_t)'B');
    rb_push(&rb, (uint8_t)'C');
    printf("after push A,B,C: count=%u empty=%u full=%u\n",
           (unsigned)rb_count(&rb), (unsigned)rb_is_empty(&rb),
           (unsigned)rb_is_full(&rb));

    /* 2) 하나 꺼내기 (FIFO 라 A 가 먼저) */
    rb_pop(&rb, &c);
    printf("pop -> %c, count=%u\n", c, (unsigned)rb_count(&rb));

    /* 3) 가득 찰 때까지 넣기 (용량 7) */
    for (x = 'D'; x <= 'L'; x++) {
        uint8_t ok = rb_push(&rb, (uint8_t)x);
        printf("push %c -> %s (count=%u)\n",
               x, ok ? "ok" : "FULL", (unsigned)rb_count(&rb));
    }

    /* 4) 전부 꺼내기 — 들어간 순서대로 나온다 */
    printf("drain: ");
    while (rb_pop(&rb, &c)) {
        putchar((int)c);
    }
    putchar('\n');

    return 0;
}
```

## PC에서 검증하기

```
gcc pc_test.c ringbuffer.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
after push A,B,C: count=3 empty=0 full=0
pop -> A, count=2
push D -> ok (count=3)
push E -> ok (count=4)
push F -> ok (count=5)
push G -> ok (count=6)
push H -> ok (count=7)
push I -> FULL (count=7)
push J -> FULL (count=7)
push K -> FULL (count=7)
push L -> FULL (count=7)
drain: BCDEFGH
```

용량이 7인 이유(크기 8 − 비움 1)와, A를 먼저 꺼냈으므로 남은 것이 B부터 시작해 `BCDEFGH` 순서로 나오는 FIFO 동작을 확인하세요.

## 자주 하는 실수

### Q. head/tail에 `volatile`을 안 붙여도 PC에선 잘 돼요.
A. PC(단일 스레드)에선 문제가 안 보일 수 있지만, ISR과 메인이 공유하는 실제 임베디드에선 컴파일러가 값을 캐싱해 갱신을 못 봅니다. 반드시 `volatile`을 붙이세요.

### Q. 크기를 10으로 했더니 wrap이 이상해요.
A. `& (N-1)` 마스킹은 N이 2의 거듭제곱일 때만 맞습니다. 10이면 `% 10`을 쓰거나, 8·16처럼 2의 거듭제곱으로 잡으세요.

### Q. 버퍼가 가득 찼는데 push가 덮어써요.
A. push에서 `((head+1)&MASK)==tail`이면 **거절(0 반환)** 해야 합니다. 안 그러면 tail을 추월해 아직 안 읽은 데이터를 덮어씁니다.

### Q. ISR에서 pop, 메인에서 push 해도 되나요?
A. 이 SPSC 안전성은 "생산자=head만, 소비자=tail만"일 때 성립합니다. 역할을 섞거나 다중 생산자가 되면 인터럽트 차단 등 추가 보호가 필요합니다.

## 정리
- 인터럽트 수신은 ISR이 버퍼에 넣고, 메인이 꺼내 처리하는 분업이 안전하다.
- 링버퍼는 head/tail로 도는 FIFO이며, 빈/가득을 인덱스로 판정한다.
- 크기를 2의 거듭제곱으로 잡으면 `& MASK`로 빠르게 wrap한다(용량은 N−1).
- SPSC에서는 head/tail 분리 + `volatile`로 락 없이 안전하다.
- UART RX ISR은 `rb_push`만, 메인은 `rb_pop` 후 처리.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[23. HAL 추상화·모듈 분리·이식성](../23_펌웨어_구조화/README.md) — 흩어진 레지스터 코드를 계층으로 나눕니다. 하드웨어 추상화(HAL)로 같은 응용 로직을 여러 칩에 이식하는 법을 배웁니다.
