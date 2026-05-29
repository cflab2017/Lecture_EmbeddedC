#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 디바운서로 PRESS/RELEASE 이벤트 검출 (THRESHOLD=2)
 *
 * 핵심 포인트:
 *  - 안정 상태와 다른 입력이 2번 연속이면 상태를 뒤집는다.
 *    같은 값(또는 안정 상태와 같은 값)이 오면 카운터를 리셋해 글리치를 거른다.
 *  - 새 상태가 1 이면 PRESS, 0 이면 RELEASE.
 *
 * 흔한 실수:
 *  - 카운터 리셋 조건을 빠뜨려 글리치가 그대로 통과하는 경우.
 *  - PRESS 와 RELEASE 모두를 누름으로 세는 경우(PRESS 만 카운트).
 */

#define THRESHOLD  2u

int main(void)
{
    uint8_t raw[12] = { 0,1,1,0,0,1,1,1,0,0,1,1 };
    uint8_t stable = 0u;
    uint8_t count  = 0u;
    int presses = 0;
    int i;

    for (i = 0; i < 12; i++) {
        uint8_t r = raw[i] ? 1u : 0u;
        if (r == stable) {
            count = 0u;
        } else {
            count++;
            if (count >= THRESHOLD) {
                stable = r;
                count = 0u;
                if (stable == 1u) {
                    printf("t=%d PRESS\n", i);
                    presses++;
                } else {
                    printf("t=%d RELEASE\n", i);
                }
            }
        }
    }

    printf("total presses = %d\n", presses);
    return 0;
}
