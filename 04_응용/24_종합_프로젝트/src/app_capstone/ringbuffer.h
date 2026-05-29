#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

/* 22편의 링버퍼 — UART RX 인터럽트와 메인 처리를 잇는 FIFO (SPSC 안전). */

#define RB_SIZE  32u            /* 2의 거듭제곱 */

typedef struct {
    uint8_t  buf[RB_SIZE];
    volatile uint16_t head;     /* 생산자(ISR) */
    volatile uint16_t tail;     /* 소비자(메인) */
} ringbuffer_t;

void     rb_init(ringbuffer_t *rb);
uint8_t  rb_push(ringbuffer_t *rb, uint8_t data);   /* 성공 1, 가득참 0 */
uint8_t  rb_pop(ringbuffer_t *rb, uint8_t *out);     /* 성공 1, 비었음 0 */

#endif /* RINGBUFFER_H */
