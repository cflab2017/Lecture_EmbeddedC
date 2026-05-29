#include <stdio.h>

/*
 * 정답 1 — 인자 있는 명령 파서
 *
 * 핵심 포인트:
 *  - "dim " 접두사를 확인한 뒤, 그 뒤 숫자를 직접 파싱한다.
 *  - 인자는 0~100 으로 클램프한다(범위 밖 입력 방어).
 *
 * 흔한 실수:
 *  - 숫자 파싱에서 자릿수 누적(v*10+digit)을 빠뜨리는 경우.
 *  - 클램프를 빼먹어 200 같은 값이 그대로 통과하는 경우.
 */

typedef enum { C_ON, C_OFF, C_DIM, C_UNKNOWN } cmd_t;

static int streq(const char *a, const char *b)
{
    while (*a != '\0' && (*a == *b)) { a++; b++; }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

static int starts_with(const char *s, const char *p)
{
    while (*p != '\0') {
        if (*s != *p) { return 0; }
        s++; p++;
    }
    return 1;
}

static int parse_uint(const char *s)
{
    int v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        s++;
    }
    return v;
}

static cmd_t parse(const char *line, int *val)
{
    *val = 0;
    if (streq(line, "on"))       { return C_ON;  }
    if (streq(line, "off"))      { return C_OFF; }
    if (starts_with(line, "dim ")) {
        int v = parse_uint(line + 4);
        if (v > 100) { v = 100; }     /* 클램프 */
        *val = v;
        return C_DIM;
    }
    return C_UNKNOWN;
}

static const char *name(cmd_t c)
{
    switch (c) {
        case C_ON:  return "ON";
        case C_OFF: return "OFF";
        case C_DIM: return "DIM";
        default:    return "UNKNOWN";
    }
}

int main(void)
{
    const char *lines[6] = { "on", "off", "dim 50", "dim 100", "dim 200", "foo" };
    int i;

    for (i = 0; i < 6; i++) {
        int v;
        cmd_t c = parse(lines[i], &v);
        printf("\"%s\" -> %s (%d)\n", lines[i], name(c), v);
    }
    return 0;
}
