#include <stdio.h>

/*
 * 정답 2 — 표시할 숫자를 자리별로 분해하고 세그먼트 코드를 붙인다.
 *
 * 핵심 포인트:
 *  - 천/백/십/일 자리는 나눗셈과 나머지로 뽑는다.
 *      천 = n/1000 % 10, 백 = n/100 % 10, 십 = n/10 % 10, 일 = n % 10
 *  - digit[0] 을 가장 왼쪽(천의 자리)으로 두면 멀티플렉싱 순서와 맞다.
 *
 * 흔한 실수:
 *  - %10 을 빼먹어 백의 자리에 20 같은 두 자리 수가 들어가는 경우.
 *  - 자리 순서(왼→오른쪽)를 뒤집어 표시가 거꾸로 나오는 경우.
 */

static const unsigned char SEG_CODE[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

int main(void)
{
    unsigned int n = 2025;     /* 표시할 값 */
    unsigned char digit[4];
    int i;

    digit[0] = (unsigned char)(n / 1000 % 10);  /* 천 */
    digit[1] = (unsigned char)(n / 100  % 10);  /* 백 */
    digit[2] = (unsigned char)(n / 10   % 10);  /* 십 */
    digit[3] = (unsigned char)(n        % 10);  /* 일 */

    for (i = 0; i < 4; i++) {
        printf("digit[%d]=%u code=0x%02X\n",
               i, (unsigned)digit[i], SEG_CODE[digit[i]]);
    }
    return 0;
}
