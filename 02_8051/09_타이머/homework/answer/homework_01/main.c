#include <reg52.h>

/*
 * 정답 1 (C51, AT89C52) — Timer1 로 지연 만들어 LED 점멸
 *
 * 핵심 포인트:
 *  - Timer1 설정은 TMOD 의 '상위 니블'에 있다(Timer0 은 하위 니블).
 *    Timer1 = Mode1 은 TMOD |= 0x10.
 *  - 제어/플래그도 Timer1 용: TR1(시작), TF1(오버플로).
 *
 * 흔한 실수:
 *  - Timer0 비트(0x01)와 Timer1 비트(0x10)를 헷갈리는 경우.
 *  - TF1 클리어를 빠뜨려 첫 대기 후 곧바로 통과해 버리는 경우.
 *
 * 예상 동작(시뮬레이터): P1.0 LED 가 약 50ms 간격으로 토글(약 10Hz).
 */

sbit LED = P1 ^ 0;

static void delay_50ms_timer1(void)
{
    TMOD &= 0x0F;          /* Timer1 설정 비트만 0 으로(Timer0 보존) */
    TMOD |= 0x10;          /* Timer1 = Mode 1 (16비트) */

    TH1 = 0x3C;            /* 65536 - 50000 = 0x3CB0 */
    TL1 = 0xB0;

    TF1 = 0;
    TR1 = 1;               /* Timer1 시작 */
    while (TF1 == 0) {     /* 50ms 경과까지 대기 */
    }
    TR1 = 0;
    TF1 = 0;
}

void main(void)
{
    while (1) {
        LED = (bit)!LED;
        delay_50ms_timer1();
    }
}
