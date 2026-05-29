#include "command.h"

/* 표준 라이브러리 없이 동작하도록 간단한 문자열 비교를 직접 둔다. */
static int streq(const char *a, const char *b)
{
    while (*a != '\0' && (*a == *b)) {
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

command_t command_parse(const char *line)
{
    if (line[0] == '\0')      { return CMD_NONE;  }
    if (streq(line, "on"))    { return CMD_ON;    }
    if (streq(line, "off"))   { return CMD_OFF;   }
    if (streq(line, "blink")) { return CMD_BLINK; }
    if (streq(line, "help"))  { return CMD_HELP;  }
    return CMD_UNKNOWN;
}

const char *command_name(command_t c)
{
    switch (c) {
        case CMD_NONE:  return "NONE";
        case CMD_ON:    return "ON";
        case CMD_OFF:   return "OFF";
        case CMD_BLINK: return "BLINK";
        case CMD_HELP:  return "HELP";
        default:        return "UNKNOWN";
    }
}
