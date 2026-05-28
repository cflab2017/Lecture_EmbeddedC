#include <stdio.h>

/*
 * 정답 2 (순수 로직, PC) — UART 8N1 프레임을 비트로 펼쳐 보기.
 * 비동기 시리얼은 한 바이트를 보낼 때 라인에 다음 순서로 비트를 싣는다:
 *   start(항상 0) → 데이터 8비트(LSB 먼저!) → stop(항상 1)
 * 패리티 없음(N), 데이터 8비트, 정지 1비트 = "8N1".
 *
 * 핵심 포인트:
 *  - 데이터는 LSB(b0)부터 나간다. MSB 부터로 착각하기 쉽다.
 *  - 평소(유휴) 라인은 High(1), start 비트만 Low(0)로 떨어뜨려 수신측이 동기를 잡는다.
 *
 * 흔한 실수:
 *  - 데이터 비트를 MSB 부터 출력해 순서가 뒤집히는 경우.
 */
int main(void)
{
    const char *msg = "Hi";
    int i, b;

    for (i = 0; msg[i] != '\0'; i++) {
        unsigned char c = (unsigned char)msg[i];
        printf("'%c' 0x%02X: 0 ", c, c);    /* start 비트 0 */
        for (b = 0; b < 8; b++) {           /* 데이터: LSB(b0)부터 */
            printf("%d", (c >> b) & 1);
        }
        printf(" 1\n");                     /* stop 비트 1 */
    }

    return 0;
}
