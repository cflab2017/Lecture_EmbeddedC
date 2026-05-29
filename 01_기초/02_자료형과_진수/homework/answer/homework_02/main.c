#include <stdio.h>
#include <stdint.h>

/*
 * 정답 2 — uint8_t 카운터 랩어라운드
 *
 * 핵심 포인트:
 *  - uint8_t 는 0~255 만 표현한다. 255 에서 1 을 더하면 256 이 아니라
 *    0 으로 '되돌아온다'(wrap-around). 하드웨어 타이머 카운터가 도는 원리다.
 *  - 250 에서 시작해 6번째 증가에서 255 → 0 으로 넘어가는 지점을 관찰한다.
 *
 * 흔한 실수:
 *  - count 를 int 로 선언하면 256, 257 ... 로 계속 커져 랩이 안 보인다.
 *    폭이 박힌 uint8_t 라야 8비트 경계에서 도는 것을 재현한다.
 *  - 255 다음을 256 으로 출력하리라 예상하는 것. unsigned 는 절대 음수가
 *    되지 않고, 최댓값을 넘으면 0 부터 다시 센다.
 */
int main(void)
{
    uint8_t count = 250;   /* 255 에 가까운 값에서 시작 */
    int step;

    printf("start: count = %u\n", (unsigned)count);

    for (step = 1; step <= 8; step++) {
        count++;           /* 255 다음에는 0 으로 랩어라운드 */
        printf("step %d: count = %u\n", step, (unsigned)count);
    }

    return 0;
}
