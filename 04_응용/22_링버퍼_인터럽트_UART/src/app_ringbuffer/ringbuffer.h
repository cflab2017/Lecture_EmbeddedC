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
