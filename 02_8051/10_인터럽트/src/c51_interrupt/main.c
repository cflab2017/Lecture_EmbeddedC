#include <reg52.h>

/*
 * 10강 예제 1 — 외부 인터럽트(INT0)로 버튼 처리.
 * 대상: AT89C52, 컴파일러: Keil C51.
 *
 * 폴링(8편)은 main 이 계속 버튼을 들여다봐야 한다. 인터럽트는 버튼이
 * 눌리는 '순간'에만 CPU 가 잠깐 끼어들어 ISR 을 실행하고, 평소 main 은
 * 자유롭다. INT0 핀(P3.2)이 하강 에지가 되면 int0_isr() 가 호출된다.
 */

sbit LED = P1 ^ 0;                       /* 액티브 로우 LED */

/* ISR 와 main 이 함께 쓰는 변수는 volatile (컴파일러 캐싱 방지). */
volatile unsigned char press_count = 0;

/* 외부 인터럽트 0 서비스 루틴: interrupt 0 (Keil C51 문법). */
void int0_isr(void) interrupt 0
{
    press_count++;          /* 눌린 횟수 누적 */
    LED = (bit)!LED;        /* LED 토글 */
}

void main(void)
{
    IT0 = 1;     /* INT0 를 '하강 에지'에서 트리거(레벨 아님) */
    EX0 = 1;     /* 외부 인터럽트 0 허용 */
    EA  = 1;     /* 전역 인터럽트 허용 (마스터 스위치) */

    while (1) {
        /* 메인은 자유. 버튼 처리는 인터럽트가 알아서 한다.
         * 필요하면 여기서 press_count 로 다른 동작을 할 수 있다. */
    }
}
