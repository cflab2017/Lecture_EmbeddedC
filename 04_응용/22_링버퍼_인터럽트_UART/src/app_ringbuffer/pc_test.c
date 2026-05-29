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
