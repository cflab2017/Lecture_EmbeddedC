#include <stdio.h>

/*
 * 정답 2 — CLAMP 매크로
 *
 * 핵심 포인트:
 *  - 함수형 매크로는 인자와 전체 식을 모두 괄호로 감싼다. 그래야 호출부에서
 *    연산자 우선순위·부호 문제로 엉뚱하게 전개되지 않는다.
 *  - CLAMP(x,lo,hi): x 가 [lo,hi] 를 벗어나면 경계값으로 잘라낸다.
 *
 * 흔한 실수:
 *  - #define CLAMP(x,lo,hi) x<lo?lo:x>hi?hi:x  처럼 괄호 없이 쓰면
 *    CLAMP(a+1, ...) 같은 호출에서 전개가 깨진다.
 *  - 매크로 인자를 부작용 있는 식(예: i++)으로 넘기면 여러 번 평가되어 위험.
 */

#define CLAMP(x, lo, hi)  ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

int main(void)
{
    printf("CLAMP(5,0,10)  = %d\n",  CLAMP(5, 0, 10));
    printf("CLAMP(15,0,10) = %d\n",  CLAMP(15, 0, 10));
    printf("CLAMP(-2,0,10) = %d\n",  CLAMP(-2, 0, 10));

    return 0;
}
