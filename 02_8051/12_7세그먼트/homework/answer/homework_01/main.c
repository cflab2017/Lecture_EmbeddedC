#include <stdio.h>

/*
 * 정답 1 — 공통 양극(common anode) 세그먼트 코드 표 만들기
 *
 * 핵심 포인트:
 *  - 공통 음극에서 비트=1 이 ON 이었다면, 공통 양극은 정반대로 비트=0 이 ON.
 *  - 따라서 양극 코드 = 음극 코드의 비트 반전(~). 8비트로 다루므로 결과를
 *    (unsigned char) 로 잘라 상위 비트 쓰레기를 없앤다.
 *
 * 흔한 실수:
 *  - ~ 결과를 int 그대로 0x%X 로 출력해 0xFFFFFFC0 처럼 나오는 경우.
 *    → (unsigned char) 캐스팅 또는 & 0xFF 마스킹으로 막는다.
 */

static const unsigned char SEG_CC[10] = {  /* 공통 음극 0~9 */
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

int main(void)
{
    int n;
    for (n = 0; n <= 9; n++) {
        unsigned char anode = (unsigned char)~SEG_CC[n];
        printf("%d -> 0x%02X\n", n, anode);
    }
    return 0;
}
