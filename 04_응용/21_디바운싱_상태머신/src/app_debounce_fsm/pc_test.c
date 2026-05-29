#include <stdio.h>
#include <stdint.h>
#include "debounce.h"

/*
 * 21강 PC 검산 — 디바운서 + LED 모드 FSM.
 * 노이즈가 낀 raw 버튼 스트림을 디바운스하고, '눌림으로 확정'되는 에지마다
 * 모드를 OFF→SLOW→FAST→OFF 로 순환시킨다.
 * 빌드: gcc pc_test.c debounce.c -o pc_test
 */

typedef enum { MODE_OFF = 0, MODE_SLOW = 1, MODE_FAST = 2 } led_mode_t;

static const char *mode_name(led_mode_t m)
{
    switch (m) {
        case MODE_OFF:  return "OFF";
        case MODE_SLOW: return "SLOW";
        case MODE_FAST: return "FAST";
        default:        return "?";
    }
}

int main(void)
{
    /* 1=눌림. 글리치(인덱스2)와 짧은 떨림(인덱스13)이 섞여 있다. */
    uint8_t raw[20] = {
        0,0,1,0,1,1,1,0,0,0,
        1,1,1,0,1,1,1,1,0,0
    };
    debounce_t db;
    led_mode_t mode = MODE_OFF;
    int i;

    debounce_init(&db, 0u);

    for (i = 0; i < 20; i++) {
        uint8_t changed = debounce_update(&db, raw[i]);
        if (changed && debounce_state(&db) == 1u) {
            mode = (led_mode_t)((mode + 1) % 3);   /* 눌림 확정 에지에서만 전진 */
        }
        printf("t=%2d raw=%u stable=%u mode=%s\n",
               i, raw[i], debounce_state(&db), mode_name(mode));
    }
    return 0;
}
