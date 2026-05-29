#include "debounce.h"

void debounce_init(debounce_t *db, uint8_t init)
{
    db->stable = init ? 1u : 0u;
    db->count  = 0u;
}

uint8_t debounce_update(debounce_t *db, uint8_t raw)
{
    uint8_t r = raw ? 1u : 0u;

    if (r == db->stable) {
        db->count = 0u;            /* 흔들림 없음 → 카운터 리셋 */
        return 0u;
    }

    db->count++;                   /* 안정 상태와 다른 입력 누적 */
    if (db->count >= DEBOUNCE_THRESHOLD) {
        db->stable = r;            /* 충분히 연속 → 상태 확정 변경 */
        db->count  = 0u;
        return 1u;                 /* 변경됨 */
    }
    return 0u;
}

uint8_t debounce_state(const debounce_t *db)
{
    return db->stable;
}
