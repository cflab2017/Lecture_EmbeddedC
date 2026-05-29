#ifndef HAL_H
#define HAL_H

#include <stdint.h>

/*
 * 하드웨어 추상화 계층(HAL) 인터페이스.
 *
 * 응용 코드는 이 함수들만 호출한다. "어떻게" 켜고 끄는지(레지스터인지
 * printf 인지)는 모른다. 구현 파일을 갈아끼우면(hal_pc.c ↔ hal_stm32.c)
 * 같은 응용 로직이 PC 에서도, 실제 보드에서도 동작한다.
 */
void hal_led_init(void);
void hal_led_set(uint8_t on);    /* 1 = 켜기, 0 = 끄기 */

#endif /* HAL_H */
