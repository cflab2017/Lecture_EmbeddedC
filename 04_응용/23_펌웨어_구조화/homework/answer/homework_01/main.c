#include <stdio.h>

/*
 * 정답 1 — 함수 포인터로 드라이버 교체
 *
 * 핵심 포인트:
 *  - 드라이버를 함수 포인터 구조체로 추상화하면, 같은 app_run 에 구현만
 *    바꿔 넘겨 동작을 교체할 수 있다(이식성의 런타임 버전).
 *  - 컴파일 타임 교체(파일 링크)와 런타임 교체(함수 포인터) 둘 다 HAL 의 방식.
 *
 * 흔한 실수:
 *  - 함수 포인터 시그니처 불일치(인자/반환형)로 호출이 깨지는 경우.
 */

typedef struct {
    void (*set)(int on);
} led_driver_t;

static void drvA_set(int on)
{
    printf("A: %s\n", on ? "ON" : "OFF");
}

static void drvB_set(int on)
{
    printf("B: [%c]\n", on ? '*' : '.');
}

static void app_run(const led_driver_t *drv)
{
    int pat[4] = { 1, 0, 1, 1 };
    int i;
    for (i = 0; i < 4; i++) {
        drv->set(pat[i]);
    }
}

int main(void)
{
    led_driver_t a = { drvA_set };
    led_driver_t b = { drvB_set };

    printf("-- driver A --\n");
    app_run(&a);
    printf("-- driver B --\n");
    app_run(&b);

    return 0;
}
