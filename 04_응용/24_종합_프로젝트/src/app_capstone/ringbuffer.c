#include "ringbuffer.h"

#define RB_MASK  (RB_SIZE - 1u)

void rb_init(ringbuffer_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
}

uint8_t rb_push(ringbuffer_t *rb, uint8_t data)
{
    uint16_t next = (uint16_t)((rb->head + 1u) & RB_MASK);
    if (next == rb->tail) {
        return 0u;               /* 가득 참 */
    }
    rb->buf[rb->head] = data;
    rb->head = next;
    return 1u;
}

uint8_t rb_pop(ringbuffer_t *rb, uint8_t *out)
{
    if (rb->head == rb->tail) {
        return 0u;               /* 비었음 */
    }
    *out = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1u) & RB_MASK);
    return 1u;
}
