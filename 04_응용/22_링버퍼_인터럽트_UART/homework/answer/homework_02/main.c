#include <stdio.h>

/*
 * 정답 2 — 수신 스트림에서 줄 단위 조립
 *
 * 핵심 포인트:
 *  - 글자를 라인 버퍼에 누적하다가 '\n' 을 만나면 한 줄로 확정해 처리한다.
 *  - 개행은 줄에 포함하지 않고, 처리 후 인덱스를 0 으로 리셋한다.
 *  - 24편에서 UART 로 받은 명령을 한 줄씩 해석하는 토대가 된다.
 *
 * 흔한 실수:
 *  - 널 종료('\0')를 빼먹어 이전 줄 찌꺼기가 출력되는 경우.
 *  - 버퍼 경계 검사를 안 해 오버플로가 나는 경우.
 */

int main(void)
{
    const char *stream = "go\nstop\nhi\n";
    char line[32];
    int idx = 0;
    const char *p;

    for (p = stream; *p != '\0'; p++) {
        char ch = *p;
        if (ch == '\n') {
            line[idx] = '\0';                  /* 줄 확정 */
            printf("LINE: %s\n", line);
            idx = 0;                           /* 다음 줄 준비 */
        } else if (idx < (int)sizeof(line) - 1) {
            line[idx++] = ch;                  /* 글자 누적(경계 검사) */
        }
    }

    return 0;
}
