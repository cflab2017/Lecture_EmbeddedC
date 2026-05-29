#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdint.h>

/*
 * 디바운서(적분기 방식).
 * 일정 주기로 raw 핀 값을 넣으면, 안정 상태와 다른 입력이 THRESHOLD 번
 * 연속될 때만 상태를 바꾼다. 짧은 글리치(채터링)는 무시된다.
 * 하드웨어 비의존이라 PC(gcc)에서 그대로 테스트할 수 있다.
 */

/* 새 입력이 이만큼 연속돼야 상태를 확정 변경한다. */
#define DEBOUNCE_THRESHOLD  3u

typedef struct {
    uint8_t stable;   /* 확정된 안정 상태 (0 또는 1) */
    uint8_t count;    /* 안정 상태와 다른 입력이 연속된 횟수 */
} debounce_t;

/* 디바운서를 초기 상태(0 또는 1)로 리셋한다. */
void debounce_init(debounce_t *db, uint8_t init);

/* raw 샘플 하나를 처리한다. 안정 상태가 바뀌면 1, 아니면 0 을 돌려준다. */
uint8_t debounce_update(debounce_t *db, uint8_t raw);

/* 현재 확정된 안정 상태(0/1). */
uint8_t debounce_state(const debounce_t *db);

#endif /* DEBOUNCE_H */
