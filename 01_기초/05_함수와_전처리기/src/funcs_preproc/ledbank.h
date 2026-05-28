#ifndef LEDBANK_H
#define LEDBANK_H

#include <stdint.h>

/*
 * ledbank.h — 8개 LED 뱅크(8비트)를 다루는 모듈의 '인터페이스'.
 * 구현은 ledbank.c 에 있고, 사용자는 이 헤더만 #include 하면 된다.
 *
 * 헤더 가드(#ifndef ~ #define ~ #endif): 같은 헤더가 여러 번 포함돼도
 * 내용이 한 번만 처리되게 막는다. 빠뜨리면 중복 정의 오류가 난다.
 */

/* 함수형 매크로 — 인자는 반드시 괄호로 감싼다(우선순위 사고 방지). */
#define BIT(n)       (1u << (n))
#define LED_COUNT    8u

/* 함수 선언(프로토타입). 반환·인자 타입만 약속하고 구현은 .c 에 둔다. */
uint8_t led_set(uint8_t bank, uint8_t pos);
uint8_t led_clear(uint8_t bank, uint8_t pos);
uint8_t led_toggle(uint8_t bank, uint8_t pos);
uint8_t led_count_on(uint8_t bank);

#endif /* LEDBANK_H */
