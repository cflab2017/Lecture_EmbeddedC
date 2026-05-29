#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — PLL 배수별 SYSCLK와 한계 검사
 *
 * 핵심 포인트:
 *  - SYSCLK = HSE × PLL배수. HSE 8MHz 기준으로 ×9 가 72MHz(표준).
 *  - F103 의 코어 클럭 한계는 72MHz. 그 이하면 OK, 초과면 OVER.
 *
 * 흔한 실수:
 *  - 72MHz 를 OVER 로 분류하는 경우. '이하(<=)'이므로 ×9(72MHz)는 OK 다.
 *  - 정렬용 %-2u 를 빼먹어 두 자리 배수(10~16)에서 화살표가 어긋나는 경우.
 */

int main(void)
{
    unsigned int mul;

    for (mul = 2; mul <= 16; mul++) {
        uint32_t sysclk = 8000000u * mul;
        const char *tag = (sysclk <= 72000000u) ? "OK" : "OVER";
        printf("PLLMUL x%-2u -> %lu Hz (%s)\n",
               mul, (unsigned long)sysclk, tag);
    }

    return 0;
}
