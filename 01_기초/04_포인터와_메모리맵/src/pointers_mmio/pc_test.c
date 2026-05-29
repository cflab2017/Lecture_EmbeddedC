#include <stdio.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

/* 주변장치를 '레지스터 묶음' 구조체로 본다 (CMSIS 가 쓰는 방식).
 * 실제론 이 구조체를 고정 주소에 '얹어서' GPIO->ODR 처럼 접근한다.
 * 멤버 순서가 곧 레지스터의 오프셋(+0,+4,+8...)이 된다. */
typedef struct {
    volatile uint32_t MODER;   /* +0x00 모드 */
    volatile uint32_t IDR;     /* +0x04 입력 */
    volatile uint32_t ODR;     /* +0x08 출력 */
} gpio_t;

/*
 * 04강 예제 2 — 레지스터 블록을 구조체+포인터로 다루는 패턴을 검증한다.
 */
int main(void)
{
    gpio_t  storage;             /* PC: 일반 변수. 보드: 0x4001_0800 같은 고정 주소 */
    gpio_t *GPIO = &storage;     /* 그 주소에 구조체를 얹는다 */

    GPIO->ODR = 0;
    GPIO->ODR |= BIT(5);         /* PA5 출력 High 라고 가정 */
    printf("ODR after set PA5 = 0x%08X\n", (unsigned)GPIO->ODR);

    GPIO->ODR ^= BIT(5);         /* toggle */
    printf("ODR after toggle  = 0x%08X\n", (unsigned)GPIO->ODR);

    /* 구조체 멤버가 +0, +4, +8 오프셋에 놓이는지 확인 */
    printf("offset MODER=%u IDR=%u ODR=%u\n",
           (unsigned)((char *)&GPIO->MODER - (char *)GPIO),
           (unsigned)((char *)&GPIO->IDR   - (char *)GPIO),
           (unsigned)((char *)&GPIO->ODR   - (char *)GPIO));

    return 0;
}
