#include <stdio.h>
#include <stdint.h>

/*
 * 02강 예제 2 — 부호 없음(unsigned) 자료형의 '랩어라운드' 와
 *               같은 비트를 부호 유무에 따라 다르게 해석하는 모습을 본다.
 * 레지스터·카운터에서 매일 만나는 현상이라 PC 에서 먼저 감을 잡는다.
 */
int main(void)
{
    uint8_t u    = 255;    /* uint8_t 최댓값 */
    uint8_t bits = 0xFF;   /* 동일한 8비트를 두 가지로 해석해 본다 */

    /* unsigned 오버플로는 '랩어라운드': 최댓값을 넘으면 0 으로 되돌아온다. */
    u = (uint8_t)(u + 1);
    printf("uint8_t 255 + 1 = %u\n", (unsigned)u);        /* 0 */

    /* 같은 0xFF 비트를 부호 없음 / 부호 있음으로 다르게 읽는다.
     * (int8_t)0xFF 는 2의 보수 환경(대부분의 MCU·PC)에서 -1 이다. */
    printf("0xFF as uint8_t = %u\n", (unsigned)bits);     /* 255 */
    printf("0xFF as int8_t  = %d\n", (int)(int8_t)bits);  /* -1  */

    return 0;
}
