#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 비트 명령 시퀀스
 *
 * 핵심 포인트:
 *  - set   : reg |=  (1u << n)
 *  - clear : reg &= ~(1u << n)
 *  - toggle: reg ^=  (1u << n)
 *    세 관용구만 알면 어떤 비트든 켜고/끄고/뒤집을 수 있다.
 *
 * 흔한 실수:
 *  - clear 에서 ~ 를 빠뜨려(reg &= (1u<<n)) 해당 비트만 남기고 나머지를
 *    모두 0 으로 지워버리는 경우. clear 는 반드시 '반전 마스크(~)' 다.
 *  - toggle 에 | 를 쓰는 경우. | 는 항상 1 로 만들 뿐 되돌리지 못한다.
 */

#define BIT(n) (1u << (n))

static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

int main(void)
{
    uint8_t reg = 0x00;
    int step = 0;

    reg |= BIT(0);                  /* set bit0    */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    reg |= BIT(2);                  /* set bit2    */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    reg |= BIT(4);                  /* set bit4    */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    reg ^= BIT(2);                  /* toggle bit2 */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    reg &= (uint8_t)~BIT(0);        /* clear bit0  */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    reg ^= BIT(4);                  /* toggle bit4 */
    printf("step %d: ", ++step); print_bin8(reg); printf("\n");

    return 0;
}
