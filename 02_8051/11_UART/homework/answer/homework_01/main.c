#include <reg52.h>

/*
 * 정답 1 (C51, AT89C52) — UART 에코(받은 문자를 되돌려 보냄, 소문자는 대문자로).
 *
 * 핵심 포인트:
 *  - 수신: RI 가 1 이 될 때까지 기다렸다가 SBUF 를 읽고 RI 를 클리어한다.
 *  - 송신: SBUF 에 쓰고 TI 가 1 이 될 때까지 기다린 뒤 TI 를 클리어한다.
 *  - 보율 생성은 Timer1 Mode2(8비트 자동 리로드), 9600bps @ 11.0592MHz → TH1=0xFD.
 *
 * 흔한 실수:
 *  - TI/RI 플래그를 클리어하지 않아 두 번째 글자부터 멈추는 경우(하드웨어가 자동으로
 *    내려주지 않는다. 소프트웨어가 0 으로 지워야 한다).
 *  - REN(SCON 의 수신 허용 비트)을 안 켜서 수신이 안 되는 경우(SCON=0x50).
 *
 * 예상 동작: μVision Serial 창에 글자를 입력하면 그대로(소문자는 대문자로) 되돌아온다.
 */

void uart_init(void)
{
    SCON = 0x50;          /* Mode1(8비트 UART) + REN(수신 허용) */
    TMOD &= 0x0F;         /* Timer1 설정 비트만 클리어(Timer0 보존) */
    TMOD |= 0x20;         /* Timer1 = Mode2 (8비트 자동 리로드) */
    TH1  = 0xFD;          /* 9600bps @ 11.0592MHz */
    TL1  = 0xFD;
    TR1  = 1;             /* Timer1 시작(보율 클럭 공급) */
}

unsigned char uart_rx(void)
{
    while (RI == 0) {     /* 수신 완료까지 대기 */
    }
    RI = 0;               /* 수신 플래그 클리어 */
    return SBUF;
}

void uart_tx(unsigned char c)
{
    SBUF = c;             /* 송신 시작 */
    while (TI == 0) {     /* 송신 완료까지 대기 */
    }
    TI = 0;               /* 송신 플래그 클리어 */
}

void main(void)
{
    unsigned char c;

    uart_init();
    while (1) {
        c = uart_rx();
        if (c >= 'a' && c <= 'z') {
            c = (unsigned char)(c - 'a' + 'A');   /* 소문자 → 대문자 */
        }
        uart_tx(c);
    }
}
