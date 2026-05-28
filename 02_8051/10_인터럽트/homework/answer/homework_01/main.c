#include <reg52.h>

/*
 * 정답 1 (C51, AT89C52) — 타이머0 인터럽트로 LED 점멸
 *
 * 핵심 포인트:
 *  - Timer0 오버플로(50ms)마다 ISR(interrupt 1)이 호출된다.
 *  - Mode1 은 자동 리로드가 아니므로 ISR 안에서 TH0/TL0 를 다시 넣는다.
 *  - main 은 인터럽트만 켜 두고 자유롭다(폴링 불필요).
 *
 * 흔한 실수:
 *  - ISR 안에서 재리로드를 빠뜨려 다음 주기가 65ms 가 아닌 ~65.5ms 로 틀어짐.
 *  - EA(전역 허용)를 안 켜서 ISR 이 아예 안 불리는 경우.
 *  - 타이머 인터럽트 번호(1)와 외부 인터럽트 번호(0)를 헷갈리는 경우.
 *
 * 예상 동작(시뮬레이터): P1.0 LED 가 약 50ms 간격으로 토글, main 은 빈 루프.
 */

sbit LED = P1 ^ 0;

void timer0_isr(void) interrupt 1
{
    TH0 = 0x3C;            /* 65536 - 50000 = 0x3CB0 재리로드 */
    TL0 = 0xB0;
    LED = (bit)!LED;       /* 50ms 마다 토글 */
}

void main(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;          /* Timer0 = Mode 1 */
    TH0 = 0x3C;
    TL0 = 0xB0;

    ET0 = 1;               /* Timer0 인터럽트 허용 */
    EA  = 1;               /* 전역 인터럽트 허용 */
    TR0 = 1;               /* Timer0 시작 */

    while (1) {
        /* 점멸은 인터럽트가 처리. main 은 다른 일을 할 수 있다. */
    }
}
