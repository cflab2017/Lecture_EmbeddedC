#include <reg52.h>

/*
 * 09강 예제 1 — 하드웨어 타이머로 정확한 지연을 만들어 LED 점멸.
 * 대상: AT89C52, 컴파일러: Keil C51. 12MHz 크리스털 가정(머신사이클 1us).
 *
 * Timer0 를 Mode 1(16비트)로 쓴다. 카운터가 0xFFFF 를 넘으면 TF0(오버플로
 * 플래그)가 1 이 된다. 50ms 뒤 오버플로가 나도록 리로드 값을 미리 넣는다:
 *   필요한 카운트 = 50ms / 1us = 50000
 *   리로드 = 65536 - 50000 = 15536 = 0x3CB0  → TH0=0x3C, TL0=0xB0
 */

sbit LED = P1 ^ 0;     /* 액티브 로우 LED */

/* Timer0 Mode1 로 약 50ms 지연(폴링 방식). */
static void delay_50ms(void)
{
    TMOD &= 0xF0;          /* Timer0 설정 비트만 0 으로(Timer1 설정은 보존) */
    TMOD |= 0x01;          /* Timer0 = Mode 1 (16비트) */

    TH0 = 0x3C;            /* 리로드 상위 바이트 */
    TL0 = 0xB0;            /* 리로드 하위 바이트 */

    TF0 = 0;               /* 오버플로 플래그 클리어 */
    TR0 = 1;               /* Timer0 시작 */
    while (TF0 == 0) {     /* 오버플로(=50ms 경과)까지 대기 */
    }
    TR0 = 0;               /* Timer0 정지 */
    TF0 = 0;               /* 다음을 위해 플래그 클리어 */
}

void main(void)
{
    while (1) {
        LED = (bit)!LED;   /* LED 토글 */
        delay_50ms();      /* 50ms 간격 → 약 10Hz 점멸 */
    }
}
