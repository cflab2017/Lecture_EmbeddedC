#include <reg52.h>

/*
 * 정답 1 (C51, AT89C52) — 포트 전체 쓰기 + sbit 개별 비트
 *
 * 핵심 포인트:
 *  - P2 = 0xAA 로 포트 전체를 한 번에 쓴 뒤, sbit 으로 한 비트씩 보정한다.
 *  - 개별 비트 쓰기는 나머지 비트를 건드리지 않는다(SFR 의 read-modify-write).
 *
 * 흔한 실수:
 *  - sbit 선언을 함수 안에 넣는 경우. sbit/sfr 은 파일 범위(전역)에 둔다.
 *  - 0xAA(1010 1010)에서 bit0 set, bit7 clear 결과를 0x2B 로 검산하지 않고
 *    엉뚱한 값을 기대하는 경우.
 *
 * 예상 동작(시뮬레이터): Port 2 창에서 P2 = 0x2B (0010 1011) 로 표시된다.
 */

sbit P2_0 = P2 ^ 0;
sbit P2_7 = P2 ^ 7;

void main(void)
{
    P2 = 0xAA;        /* 1010 1010 */

    P2_0 = 1;         /* bit0 set   → 1010 1011 = 0xAB */
    P2_7 = 0;         /* bit7 clear → 0010 1011 = 0x2B */

    while (1) {
        /* P2 = 0x2B 유지 */
    }
}
