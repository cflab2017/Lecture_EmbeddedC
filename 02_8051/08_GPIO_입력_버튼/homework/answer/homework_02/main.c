#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — 버튼 누름 횟수 세기
 *
 * 핵심 포인트:
 *  - '누름'은 상태가 1→0 으로 바뀌는 하강 에지다. 그 순간만 센다.
 *  - 계속 누르고 있어도(0,0,0...) 1번으로 세어야 하므로 에지로 판단한다.
 *
 * 흔한 실수:
 *  - btn==0 인 횟수를 그대로 세는 경우. 그러면 길게 누르면 여러 번 세진다.
 *    반드시 '직전 1, 현재 0'인 순간만 센다.
 */
int main(void)
{
    unsigned char seq[] = { 1, 1, 0, 1, 0, 0, 1, 0 };  /* 시간순 입력 */
    int n = (int)(sizeof(seq) / sizeof(seq[0]));
    unsigned char prev = 1;
    int presses = 0;
    int t;

    for (t = 0; t < n; t++) {
        if (prev == 1 && seq[t] == 0) {   /* 하강 에지 */
            presses++;
        }
        prev = seq[t];
    }

    printf("presses = %d\n", presses);   /* 3 */

    return 0;
}
