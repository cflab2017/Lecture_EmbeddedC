#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — 두 칸짜리 불빛이 흐르는 패턴
 *
 * 핵심 포인트:
 *  - 인접한 두 LED 를 동시에 켠다: 마스크 (1<<pos) | (1<<(pos+1)).
 *  - 액티브 로우라 포트 값은 그 마스크의 반전: ~mask.
 *
 * 흔한 실수:
 *  - 반전(~)을 빼먹어 켜질 LED 와 꺼질 LED 가 뒤바뀌는 경우.
 *  - pos 가 6 을 넘어 (1<<8) 로 포트 밖을 가리키는 경우(0~6 까지만).
 */
int main(void)
{
    unsigned char pos;

    for (pos = 0; pos < 7; pos++) {
        unsigned int mask = (1u << pos) | (1u << (pos + 1));
        unsigned char port = (unsigned char)~mask;
        printf("pos %u: 0x%02X\n", (unsigned)pos, port);
    }

    return 0;
}
