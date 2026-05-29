#ifndef COMMAND_H
#define COMMAND_H

/*
 * 명령 파서 — 한 줄 문자열을 명령 코드로 바꾼다.
 * 하드웨어 비의존 순수 로직이라 PC(gcc)에서 단위 테스트할 수 있다.
 */

typedef enum {
    CMD_NONE = 0,   /* 빈 줄 */
    CMD_ON,
    CMD_OFF,
    CMD_BLINK,
    CMD_HELP,
    CMD_UNKNOWN
} command_t;

command_t   command_parse(const char *line);
const char *command_name(command_t c);

#endif /* COMMAND_H */
