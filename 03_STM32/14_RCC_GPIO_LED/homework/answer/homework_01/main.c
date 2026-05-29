#include <stdio.h>
#include <stdint.h>

/*
 * 정답 1 — F1 GPIO 4비트 설정 코드 계산
 *
 * 핵심 포인트:
 *  - 핀 하나의 설정 4비트 = (CNF << 2) | MODE.
 *  - MODE: 00 입력, 01/10/11 출력(10/2/50MHz). CNF 의미는 입력/출력에 따라 다름.
 *
 * 흔한 실수:
 *  - MODE 와 CNF 자리를 뒤집는 경우(하위 2비트가 MODE, 그 위 2비트가 CNF).
 */

static unsigned int gpio_code(unsigned int mode, unsigned int cnf)
{
    return (cnf << 2) | mode;     /* [3:2]=CNF, [1:0]=MODE */
}

int main(void)
{
    unsigned int mode[5] = { 0u, 2u, 3u, 2u, 0u };
    unsigned int cnf[5]  = { 1u, 0u, 0u, 1u, 2u };
    int i;

    for (i = 0; i < 5; i++) {
        printf("mode=%u cnf=%u -> 0x%X\n",
               mode[i], cnf[i], gpio_code(mode[i], cnf[i]));
    }
    return 0;
}
