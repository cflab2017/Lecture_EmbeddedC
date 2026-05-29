#include <stdio.h>

/*
 * 정답 1 — EXTI 라인 → EXTICR 인덱스/시프트 계산
 *
 * 핵심 포인트:
 *  - AFIO->EXTICR 는 4개 배열이고, 각 레지스터가 라인 4개를 4비트씩 담는다.
 *  - 라인 n: 인덱스 = n/4, 시프트 = (n%4)*4.
 *
 * 흔한 실수:
 *  - %4 와 /4 를 뒤바꾸는 경우. 인덱스는 나눗셈, 시프트는 나머지×4.
 */

int main(void)
{
    int lines[5] = { 0, 3, 4, 7, 13 };
    int i;

    for (i = 0; i < 5; i++) {
        int n = lines[i];
        int idx = n / 4;
        int shift = (n % 4) * 4;
        printf("EXTI%d -> EXTICR[%d], shift %d\n", n, idx, shift);
    }
    return 0;
}
