#include <reg52.h>

/*
 * 12강 예제 — 4자리 7-세그먼트 동적 구동(멀티플렉싱).
 * 대상: AT89C52, 컴파일러: Keil C51.
 *
 * 배선 가정(공통 음극, common cathode):
 *   - 세그먼트 a~g, dp 는 P2 에 연결. 비트=1 이면 그 세그먼트 ON.
 *       a=P2.0  b=P2.1  c=P2.2  d=P2.3  e=P2.4  f=P2.5  g=P2.6  dp=P2.7
 *   - 자리 선택은 P0 하위 4비트. 비트=1 이면 그 자리를 켠다(자리당 1비트).
 *       digit0=P0.0  digit1=P0.1  digit2=P0.2  digit3=P0.3
 *
 * 멀티플렉싱: 한 순간에는 한 자리만 켜고, 4자리를 아주 빠르게 돌며 켠다.
 * 사람 눈의 잔상(persistence of vision) 덕분에 4자리가 동시에 켜진 것처럼 보인다.
 */

#define SEG_PORT  P2     /* 세그먼트 패턴 출력 */
#define DIG_PORT  P0     /* 자리 선택 */

/* 공통 음극용 0~9 세그먼트 코드 (비트=1 → 세그먼트 ON).
 * 비트 순서: a(0) b(1) c(2) d(3) e(4) f(5) g(6) dp(7) */
static const unsigned char SEG_CODE[10] = {
    0x3F, /* 0 */ 0x06, /* 1 */ 0x5B, /* 2 */ 0x4F, /* 3 */ 0x66, /* 4 */
    0x6D, /* 5 */ 0x7D, /* 6 */ 0x07, /* 7 */ 0x7F, /* 8 */ 0x6F  /* 9 */
};

/* 짧은 소프트웨어 지연(자리 유지 시간). 정확한 시간은 9편 타이머 참고. */
static void delay(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) {
        /* 시간 보내기 */
    }
}

void main(void)
{
    /* 표시할 4자리. digits[0] 이 가장 왼쪽(자리0). 여기서는 "1234". */
    unsigned char digits[4] = { 1, 2, 3, 4 };
    unsigned char d;

    while (1) {
        for (d = 0; d < 4; d++) {
            DIG_PORT = 0x00;                       /* 1) 모든 자리 끄기(잔상 방지) */
            SEG_PORT = SEG_CODE[digits[d]];        /* 2) 이 자리의 세그먼트 패턴 */
            DIG_PORT = (unsigned char)(1u << d);   /* 3) 이 자리만 켜기 */
            delay(150);                            /* 4) 잠깐 유지 후 다음 자리 */
        }
    }
}
