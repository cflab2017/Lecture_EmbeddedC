#include <stdio.h>
#include <stdint.h>

/*
 * 15강 PC 검산 — 풀업 입력에서 버튼 눌림 판정과 LED 결정 로직.
 * 풀업 + 버튼이 GND 로 연결되면, 안 누르면 1(풀업), 누르면 0(액티브 로우).
 * 하드웨어 없이 여러 IDR 값에 대한 판정을 확인한다.
 */

static int is_pressed(uint32_t idr, unsigned int pin)
{
    return ((idr & (1u << pin)) == 0u) ? 1 : 0;   /* 비트가 0 이면 눌림 */
}

int main(void)
{
    uint32_t samples[4] = { 0x0001u, 0x0000u, 0x0021u, 0x0020u };
    unsigned int pin = 0u;
    int i;

    for (i = 0; i < 4; i++) {
        int pressed = is_pressed(samples[i], pin);
        printf("IDR=0x%04lX -> %s -> LED %s\n",
               (unsigned long)samples[i],
               pressed ? "PRESSED" : "released",
               pressed ? "ON" : "OFF");
    }
    return 0;
}
