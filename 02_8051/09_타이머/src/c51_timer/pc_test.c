#include <stdio.h>

/*
 * 09강 예제 2 — 타이머 리로드 값 계산을 PC 에서 검증한다.
 * 12MHz(머신사이클 1us) 기준, 원하는 지연(us)에 필요한 16비트 리로드와
 * TH/TL 바이트를 구한다. Mode1 은 16비트라 1us 단위로 최대 65536us.
 *   reload = 65536 - us,  TH = reload>>8,  TL = reload & 0xFF
 */
int main(void)
{
    unsigned int delays_us[] = { 1000, 5000, 50000 };
    int n = (int)(sizeof(delays_us) / sizeof(delays_us[0]));
    int k;

    for (k = 0; k < n; k++) {
        unsigned int us = delays_us[k];
        unsigned int reload = (unsigned int)(65536UL - us);
        unsigned char th = (unsigned char)(reload >> 8);
        unsigned char tl = (unsigned char)(reload & 0xFF);
        printf("%5u us -> reload=%5u  TH0=0x%02X TL0=0x%02X\n",
               us, reload, th, tl);
    }

    return 0;
}
