#include <stdio.h>

/*
 * 06강 예제 2 — 강의 예제(main.c)가 만드는 최종 포트 값을 PC 에서 검산한다.
 * 실제 칩에서는 P1 이 SFR(0x90 번지)이지만, 여기서는 일반 변수로 흉내내
 * "P1 = 0x0F 후 P1.0 을 0, P1.7 을 1 로" 했을 때의 결과만 확인한다.
 */
int main(void)
{
    unsigned char P1 = 0x0F;          /* 포트 전체 쓰기 */

    P1 = (unsigned char)(P1 & ~(1u << 0));   /* LED0 = 0 → P1.0 clear */
    P1 = (unsigned char)(P1 | (1u << 7));    /* LED7 = 1 → P1.7 set   */

    printf("P1 = 0x%02X\n", P1);      /* 0x8E */

    return 0;
}
