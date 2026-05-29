#include <stdio.h>

/*
 * 정답 2 — 4단계 모드 FSM
 *
 * 핵심 포인트:
 *  - 상태를 enum 으로 명시하고, 누름 이벤트마다 (state+1)%4 로 순환.
 *  - HIGH(3) 다음은 OFF(0) 로 자연스럽게 감싼다.
 *
 * 흔한 실수:
 *  - %4 를 빼먹어 상태가 4 이상으로 넘어가 이름 배열을 벗어나는 경우.
 *  - press 가 0 일 때도 전진시키는 경우(누름이 있을 때만 전진).
 */

typedef enum { ST_OFF = 0, ST_LOW = 1, ST_MID = 2, ST_HIGH = 3 } mode_t;

static const char *name_of(mode_t m)
{
    static const char *names[4] = { "OFF", "LOW", "MID", "HIGH" };
    return names[m];
}

int main(void)
{
    int press[8] = { 1, 0, 1, 1, 0, 1, 0, 1 };
    mode_t state = ST_OFF;
    int i;

    for (i = 0; i < 8; i++) {
        if (press[i]) {
            state = (mode_t)((state + 1) % 4);
        }
        printf("t=%d press=%d state=%s\n", i, press[i], name_of(state));
    }
    return 0;
}
