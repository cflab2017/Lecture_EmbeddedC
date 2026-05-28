#include <stdio.h>
#include <stdint.h>

/*
 * 정답 2 — 두 개의 LED 교대 점멸
 *
 * 핵심 포인트:
 *  - B를 따로 관리하지 않고 b = !a 로 매번 A의 반대로 계산한다.
 *    두 상태가 어긋날 가능성을 원천 차단하는 방법이다.
 *  - 출력 정렬: A는 "%-3s"로 폭 3에 왼쪽 정렬("ON "/"OFF")해 콤마 위치를
 *    맞춘다. B는 줄 끝이라 패딩이 필요 없다.
 *
 * 흔한 실수:
 *  - A와 B를 둘 다 따로 토글하다가 동시에 같은 상태가 되는 버그.
 *  - 정렬용 공백을 빠뜨려 예상 출력과 글자 단위로 어긋나는 경우.
 */
int main(void)
{
    uint8_t a = 1;   /* 시작: A = ON */
    uint8_t b;
    uint8_t tick;

    for (tick = 0; tick < 6; tick++) {
        b = (uint8_t)!a;                /* B는 항상 A의 반대 */
        printf("tick %u: A = %-3s, B = %s\n",
               (unsigned)tick,
               a ? "ON" : "OFF",
               b ? "ON" : "OFF");
        a = (uint8_t)!a;                /* 다음 tick을 위해 A 토글 */
    }

    return 0;
}
