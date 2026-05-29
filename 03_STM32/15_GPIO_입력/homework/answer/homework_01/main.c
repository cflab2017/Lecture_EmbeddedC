#include <stdio.h>

/*
 * 정답 1 — 버튼 누름(상승 에지)에서 LED 토글
 *
 * 핵심 포인트:
 *  - released(0) → pressed(1) 로 바뀌는 '상승 에지'에서만 LED 를 뒤집는다.
 *  - 단순 '눌려 있음'이 아니라 '새로 눌린 순간'을 잡는 것이 핵심
 *    (21편 디바운싱·FSM 의 씨앗).
 *
 * 흔한 실수:
 *  - 눌려 있는 동안 매 샘플 토글해서 LED 가 떨리는 경우 → 에지에서만 토글.
 *  - prev 갱신을 빠뜨려 에지를 못 잡는 경우.
 */

int main(void)
{
    int samples[8] = { 0, 1, 1, 0, 1, 0, 0, 1 };
    int prev = 0;       /* 직전 눌림 상태 */
    int led  = 0;       /* 0=OFF, 1=ON */
    int i;

    for (i = 0; i < 8; i++) {
        int pressed = samples[i];
        if (pressed && !prev) {     /* 상승 에지 */
            led = !led;
        }
        prev = pressed;
        printf("sample %d: pressed=%d LED=%s\n",
               i, pressed, led ? "ON" : "OFF");
    }
    return 0;
}
