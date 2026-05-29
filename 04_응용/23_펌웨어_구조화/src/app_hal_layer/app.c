#include "app.h"
#include "hal.h"

/*
 * 응용 계층 — HAL 만 호출한다. 여기에는 레지스터 코드가 한 줄도 없다.
 * 그래서 이 파일은 칩이 바뀌어도 수정할 필요가 없다(이식성의 핵심).
 */
void app_blink_pattern(const uint8_t *pattern, uint16_t len)
{
    uint16_t i;

    hal_led_init();
    for (i = 0; i < len; i++) {
        hal_led_set(pattern[i] ? 1u : 0u);
    }
}
