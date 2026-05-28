#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — 포트 값 변화를 단계별로 검산
 *
 * 핵심 포인트:
 *  - 포트 SFR 을 일반 변수로 흉내내, 비트 조작 결과를 단계별로 확인한다.
 *  - 상위 니블 set(|= 0xF0), 특정 비트 clear/toggle 의 누적 결과를 본다.
 *
 * 흔한 실수:
 *  - 각 단계가 누적임을 잊고 매번 0x00 에서 다시 시작하는 경우.
 */
int main(void)
{
    unsigned char p = 0x00;

    p = (unsigned char)(p | 0xF0);          /* 상위 니블 set  */
    printf("step1 (set high nibble): 0x%02X\n", p);   /* 0xF0 */

    p = (unsigned char)(p & ~(1u << 4));    /* bit4 clear     */
    printf("step2 (clear bit4)     : 0x%02X\n", p);   /* 0xE0 */

    p = (unsigned char)(p ^ (1u << 0));     /* bit0 toggle    */
    printf("step3 (toggle bit0)    : 0x%02X\n", p);   /* 0xE1 */

    return 0;
}
