#include <reg52.h>

/*
 * 11강 예제 1 — UART 로 PC 에 문자열 보내기(9600bps, 8N1).
 * 대상: AT89C52, 컴파일러: Keil C51. UART 용 11.0592MHz 크리스털 가정.
 *
 * 비동기 시리얼(UART)은 클럭선 없이, 양쪽이 약속한 '보율(baud)'로 비트를
 * 주고받는다. 8051 은 Timer1 의 오버플로로 보율 클럭을 만든다.
 *   9600bps @ 11.0592MHz, SMOD=0 → TH1 = 256 - 11059200/(384*9600) = 0xFD
 *
 * 송신 흐름: SBUF 에 한 바이트를 쓰면 하드웨어가 비트를 밀어내고, 다 보내면
 * TI(송신 완료 플래그)를 1 로 세운다. 소프트웨어가 TI 를 기다렸다 0 으로 지운다.
 */

/* UART 를 9600bps, 8비트, 정지 1, 패리티 없음(8N1)으로 초기화. */
void uart_init(void)
{
    SCON = 0x50;          /* SM1=1(Mode1, 8비트 UART) + REN=1(수신 허용) */
    TMOD &= 0x0F;         /* Timer1 비트만 클리어(Timer0 설정 보존) */
    TMOD |= 0x20;         /* Timer1 = Mode2(8비트 자동 리로드) → 보율 생성 */
    TH1  = 0xFD;          /* 리로드 값: 9600bps @ 11.0592MHz */
    TL1  = 0xFD;
    TR1  = 1;             /* Timer1 시작 */
}

/* 한 바이트 송신(완료까지 대기). */
void uart_tx(unsigned char c)
{
    SBUF = c;             /* 보낼 바이트를 버퍼에 쓰면 송신 시작 */
    while (TI == 0) {     /* 송신 완료(TI=1)까지 대기 */
    }
    TI = 0;               /* 다음 송신을 위해 플래그 클리어 */
}

/* 널 종료 문자열 송신. */
void uart_puts(const char *s)
{
    while (*s) {
        uart_tx((unsigned char)*s++);
    }
}

/* 관찰하기 좋게 송신 사이에 잠깐 쉰다(정밀할 필요 없는 소프트웨어 지연). */
static void delay(void)
{
    unsigned int i;
    for (i = 0; i < 60000; i++) {
    }
}

void main(void)
{
    uart_init();
    while (1) {
        uart_puts("Hello, 8051!\r\n");   /* Serial 창에서 한 줄씩 보인다 */
        delay();
    }
}
