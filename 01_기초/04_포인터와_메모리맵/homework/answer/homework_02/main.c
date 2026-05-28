#include <stdio.h>
#include <stdint.h>

/*
 * 정답 2 — 포인터로 두 값 교환(swap)
 *
 * 핵심 포인트:
 *  - 함수가 호출자의 변수를 바꾸려면 '주소'를 받아야 한다(포인터 전달).
 *  - 값으로 전달하면 복사본만 바뀌어 원본은 그대로다.
 *
 * 흔한 실수:
 *  - swap(a, b) 처럼 값으로 넘겨 원본이 안 바뀌는 경우. swap(&a, &b) 로
 *    주소를 넘기고, 함수 안에서 *pa, *pb 로 역참조해 교환한다.
 */

static void swap_u8(uint8_t *pa, uint8_t *pb)
{
    uint8_t tmp = *pa;
    *pa = *pb;
    *pb = tmp;
}

int main(void)
{
    uint8_t a = 0xAB;
    uint8_t b = 0xCD;

    printf("before: a = 0x%02X, b = 0x%02X\n", a, b);
    swap_u8(&a, &b);
    printf("after : a = 0x%02X, b = 0x%02X\n", a, b);

    return 0;
}
