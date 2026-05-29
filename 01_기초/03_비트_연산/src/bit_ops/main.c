#include <stdio.h>
#include <stdint.h>

/* 비트 위치에 의미 있는 이름을 준다 (매직 넘버 금지). */
#define LED_RED     0u   /* bit 0 */
#define LED_GREEN   1u   /* bit 1 */
#define LED_BLUE    2u   /* bit 2 */

#define BIT(n)      (1u << (n))   /* n 번 비트만 1 인 마스크 */

/* 8비트 값을 2진수 8자리로 출력한다. */
static void print_bin8(uint8_t v)
{
    int i;
    for (i = 7; i >= 0; i--) {
        putchar(((v >> i) & 1u) ? '1' : '0');
    }
}

/* 라벨과 현재 레지스터 상태(2진)를 한 줄로 출력한다. */
static void show(const char *label, uint8_t reg)
{
    printf("%-12s: ", label);
    print_bin8(reg);
    printf("\n");
}

/*
 * 03강 예제 1 — 8비트 '포트' 레지스터를 비트 단위로 조작한다.
 * 실제 보드라면 reg 가 GPIO 출력 레지스터이고, 각 비트가 LED 한 개다.
 * set( |= ), clear( &= ~ ), toggle( ^= ), test( >> & ) 네 관용구를 본다.
 */
int main(void)
{
    uint8_t reg = 0x00;            /* 모든 LED OFF 로 시작 */

    show("init", reg);

    reg |= BIT(LED_RED);           /* set   : RED 켜기 */
    show("set RED", reg);

    reg |= BIT(LED_BLUE);          /* set   : BLUE 켜기 */
    show("set BLUE", reg);

    reg ^= BIT(LED_RED);           /* toggle: RED 뒤집기(켜져 있으니 꺼짐) */
    show("toggle RED", reg);

    reg &= (uint8_t)~BIT(LED_BLUE);/* clear : BLUE 끄기 */
    show("clear BLUE", reg);

    /* test: GREEN 비트가 켜져 있는가? */
    printf("GREEN is %s\n", ((reg >> LED_GREEN) & 1u) ? "ON" : "OFF");

    return 0;
}
