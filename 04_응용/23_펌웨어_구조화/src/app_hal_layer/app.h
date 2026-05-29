#ifndef APP_H
#define APP_H

#include <stdint.h>

/*
 * 응용 로직. HAL 인터페이스만 사용하며 하드웨어에 의존하지 않는다.
 * pattern 의 각 원소(0/1)대로 LED 를 순서대로 설정한다.
 */
void app_blink_pattern(const uint8_t *pattern, uint16_t len);

#endif /* APP_H */
