#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — 블링크 횟수와 ON 카운트
 *
 * 핵심 포인트:
 *  - 시작 상태 0에서 토글하면 첫 tick(0)은 ON이 된다.
 *  - LED가 ON으로 바뀔 때마다 on_count 를 1 증가시킨다.
 *
 * 흔한 실수:
 *  - 토글 '전'에 상태를 출력해 한 칸씩 밀리는 경우. 이 문제는 "토글한 뒤
 *    그 상태를 출력"하는 순서다. (토글 → 카운트 → 출력)
 *  - 카운터를 토글 횟수(8)로 착각하는 경우. 세는 것은 'ON이었던' 횟수다.
 */
int main(void)
{
    uint8_t led = 0;        /* 0 = 꺼짐, 1 = 켜짐 */
    uint8_t on_count = 0;   /* LED가 ON이었던 횟수 */
    uint8_t tick;

    for (tick = 0; tick < 8; tick++) {
        led = (uint8_t)!led;            /* 먼저 토글 */
        if (led) {
            on_count++;                 /* ON이면 카운트 */
        }
        printf("tick %u: LED = %s\n",
               (unsigned)tick, led ? "ON" : "OFF");
    }

    printf("ON count = %u\n", (unsigned)on_count);

    return 0;
}
