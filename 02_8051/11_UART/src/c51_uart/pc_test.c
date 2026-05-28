#include <stdio.h>

/*
 * 11강 예제 2 — UART 보율(baud)에 맞는 Timer1 리로드(TH1) 계산 검증.
 * 11.0592MHz 크리스털, SMOD=0, Timer1 Mode2(8비트 자동 리로드) 기준.
 *   n   = Fosc / (384 * baud)   (Timer1 이 한 번 오버플로될 때까지 세는 수)
 *   TH1 = 256 - n
 * 11.0592MHz 는 표준 보율에서 n 이 '정수'로 떨어져 보율 오차가 0 이다. 그래서
 * UART 용 크리스털로 12MHz 대신 11.0592MHz 를 즐겨 쓴다(12MHz 는 9600 에서 약 8.5%
 * 오차가 나 통신이 깨진다).
 */
int main(void)
{
    unsigned long fosc = 11059200UL;       /* 11.0592 MHz */
    unsigned int bauds[] = { 1200, 2400, 4800, 9600 };
    int count = (int)(sizeof(bauds) / sizeof(bauds[0]));
    int k;

    for (k = 0; k < count; k++) {
        unsigned int baud = bauds[k];
        unsigned int n = (unsigned int)(fosc / (384UL * baud));
        unsigned char th1 = (unsigned char)(256 - n);
        printf("%5u baud -> n=%3u  TH1=0x%02X\n", baud, n, th1);
    }

    return 0;
}
