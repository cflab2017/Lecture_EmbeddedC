#include <stdio.h>
#include "command.h"

/*
 * 24강 PC 검산 — 명령 파서 단위 테스트.
 * 입력 한 줄을 명령 코드로 바꾸는 로직만 하드웨어 없이 검증한다.
 * 빌드: gcc pc_test.c command.c -o pc_test
 */

int main(void)
{
    const char *lines[6] = { "on", "off", "blink", "help", "xyz", "" };
    int i;

    for (i = 0; i < 6; i++) {
        printf("\"%s\" -> %s\n", lines[i], command_name(command_parse(lines[i])));
    }
    return 0;
}
