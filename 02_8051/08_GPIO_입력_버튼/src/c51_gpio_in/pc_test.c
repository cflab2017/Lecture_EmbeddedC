#include <stdio.h>

/*
 * 08강 예제 2 — 누르는 '순간'마다 LED 를 토글하는 에지 검출 로직(PC 검증).
 * 버튼 상태 1=뗌, 0=눌림. 직전 상태와 비교해 1→0(하강 에지)일 때만 토글.
 * 실제 칩에서는 BTN 을 주기적으로 읽어 이 로직을 적용한다.
 */

static unsigned char update_led(unsigned char led,
                                unsigned char prev_btn,
                                unsigned char btn)
{
    if (prev_btn == 1 && btn == 0) {   /* 하강 에지 = 눌린 순간 */
        led = (unsigned char)!led;
    }
    return led;
}

int main(void)
{
    unsigned char seq[] = { 1, 1, 0, 0, 1, 0, 1, 0, 0, 1 };  /* 시간순 입력 */
    int n = (int)(sizeof(seq) / sizeof(seq[0]));
    unsigned char led = 0;
    unsigned char prev = 1;
    int t;

    for (t = 0; t < n; t++) {
        led = update_led(led, prev, seq[t]);
        printf("t=%d btn=%u led=%u\n", t, (unsigned)seq[t], (unsigned)led);
        prev = seq[t];
    }

    return 0;
}
