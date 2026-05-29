#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 덮어쓰기(overwrite) 링버퍼
 *
 * 핵심 포인트:
 *  - 가득 차면 거절하는 대신, 가장 오래된 값(tail 위치)을 버리고 새 값을 넣는다.
 *    "최근 N개만 유지"하는 로그 버퍼에 흔한 정책.
 *  - 버릴 때 tail 을 한 칸 전진시킨 뒤 head 에 새 값을 쓴다.
 *
 * 흔한 실수:
 *  - 가득일 때 tail 전진을 빠뜨려 head 가 tail 을 추월하는 경우.
 *  - 크기가 2의 거듭제곱이 아니어서 & MASK wrap 이 깨지는 경우.
 */

#define SZ    4u
#define MASK  (SZ - 1u)

static uint8_t  buf[SZ];
static uint16_t head = 0u, tail = 0u;

static int is_full(void)
{
    return (((head + 1u) & MASK) == tail) ? 1 : 0;
}

/* 덮어썼으면 버린 문자를 *dropped 에, 아니면 0 을 넣는다. */
static void push_ow(uint8_t d, uint8_t *dropped)
{
    if (is_full()) {
        *dropped = buf[tail];
        tail = (tail + 1u) & MASK;     /* 가장 오래된 것 폐기 */
    } else {
        *dropped = 0u;
    }
    buf[head] = d;
    head = (head + 1u) & MASK;
}

static int pop(uint8_t *out)
{
    if (head == tail) {
        return 0;
    }
    *out = buf[tail];
    tail = (tail + 1u) & MASK;
    return 1;
}

int main(void)
{
    char x;
    uint8_t dropped, c;

    for (x = 'A'; x <= 'E'; x++) {
        push_ow((uint8_t)x, &dropped);
        if (dropped) {
            printf("push %c -> overwrite (dropped %c)\n", x, dropped);
        } else {
            printf("push %c -> ok\n", x);
        }
    }

    printf("drain: ");
    while (pop(&c)) {
        putchar((int)c);
    }
    putchar('\n');

    return 0;
}
