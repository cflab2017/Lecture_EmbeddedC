# 16. SysTick 타이머와 정확한 지연

지금까지 쓴 `delay()`는 빈 루프를 도는 방식이라 **시간이 부정확**합니다. 컴파일러 최적화, 클럭 속도, 인터럽트에 따라 실제 지연이 들쭉날쭉하죠. 이번 편은 Cortex-M 코어에 내장된 **SysTick** 타이머로 이를 해결합니다. SysTick은 모든 Cortex-M에 똑같이 들어 있는 24비트 카운터라, 칩이 바뀌어도 같은 코드가 동작하는 이식성도 얻습니다.

핵심 패턴은 "**1ms마다 인터럽트 → 전역 밀리초 카운터 증가 → 그 카운터로 시간 측정**"입니다. 이 한 패턴으로 정확한 지연, 주기 실행, 타임아웃을 모두 구현할 수 있습니다.

## 학습 목표
- SysTick의 구조(24비트 다운 카운터)와 동작을 이해한다.
- `SysTick_Config()`로 1ms 주기 인터럽트를 설정한다.
- `SysTick_Handler`에서 밀리초 카운터를 증가시킨다.
- 부호 없는 뺄셈으로 래핑에 안전한 `delay_ms()`를 만든다.
- SysTick의 24비트 한계와 reload 계산을 안다.

## 대상 환경
- 컴파일러: **Keil MDK-ARM (Arm Compiler 6)**.
- 디바이스: **STM32F103C8** (μVision 시뮬레이터).
- 검증: μVision 시뮬레이터 (Watch로 `g_ms`, Logic Analyzer PC13). 시간 계산은 PC `gcc`(`pc_test.c`).

## 핵심 개념

### 1) SysTick이란
SysTick은 Cortex-M 코어 안에 있는 **24비트 다운 카운터**입니다. reload 값에서 0까지 내려가고, 0에 닿으면 다시 reload 값으로 채워지며 인터럽트(SysTick exception)를 발생시킬 수 있습니다. 코어에 내장돼 있어 어떤 STM32(또는 다른 Cortex-M)에서도 같은 방식으로 씁니다.

### 2) `SysTick_Config()`
CMSIS가 제공하는 헬퍼 한 줄로 설정이 끝납니다.

```c
SysTick_Config(SystemCoreClock / 1000u);   /* 1ms 마다 인터럽트 */
```

이 함수는 내부에서 ① reload 레지스터에 `ticks - 1`을 쓰고, ② SysTick 인터럽트를 켜고, ③ 코어 클럭을 소스로 카운터를 시작합니다. 인자는 "몇 클럭마다 인터럽트를 낼지"이므로, 72MHz에서 `72000000/1000 = 72000`이면 1ms입니다.

### 3) `SysTick_Handler`와 밀리초 카운터
SysTick 인터럽트의 핸들러 이름은 CMSIS startup에 `SysTick_Handler`로 약속돼 있습니다. 여기서 전역 카운터를 1 늘립니다.

```c
static volatile uint32_t g_ms = 0;
void SysTick_Handler(void) { g_ms++; }
```

`volatile`이 필수입니다. 핸들러(인터럽트)와 메인 루프가 같은 변수를 공유하므로, 컴파일러가 캐싱하지 못하게 막아야 합니다.

### 4) 래핑에 안전한 `delay_ms()`
```c
static void delay_ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) { }
}
```

`g_ms`가 약 49.7일 후 32비트 한계를 넘어 0으로 돌아가도, **부호 없는 뺄셈** `g_ms - start`는 올바른 경과 시간을 줍니다(실제 경과가 2³² 이내인 한). 이 트릭을 `pc_test.c`로 직접 확인합니다.

### 5) 24비트 한계
SysTick 카운터는 24비트라 reload 최댓값은 `0xFFFFFF = 16,777,215`입니다. 72MHz에서 한 주기 최대 약 **233ms**(16,777,216/72MHz)입니다. 그보다 긴 주기는 SysTick 한 번으로 못 만들고, 1ms 틱을 세는 방식(이 편 패턴)으로 해결합니다.

## μVision 프로젝트 만들기
1. `Project → New µVision Project…` → 이름 `stm32_systick`.
2. Device: **STM32F103C8**.
3. `Manage Run-Time Environment` → **CMSIS:CORE**, **Device:Startup**.
4. `main.c` 추가 후 복붙.
5. `C/C++ → Define`에 `STM32F10X_MD`.
6. `Debug → Use Simulator`, `-pSTM32F103C8`.
7. Rebuild(F7) → Debug(Ctrl+F5).

## 예제로 보기

### 예제 1 — `main.c` : SysTick으로 정확한 500ms 점멸
```c
#include "stm32f10x.h"

/*
 * 16강 예제 — SysTick 으로 정확한 500ms 점멸.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * SysTick 은 Cortex-M 코어에 내장된 24비트 다운 카운터다.
 * SysTick_Config(N) 이 reload=N-1 로 맞추고 인터럽트를 켜며 시작시킨다.
 * SystemCoreClock/1000 을 넣으면 1ms 마다 SysTick_Handler 가 불린다.
 */

#define LED_PIN   13u    /* PC13 */

static volatile uint32_t g_ms = 0;   /* 1ms 마다 증가하는 시스템 틱 */

void SysTick_Handler(void)
{
    g_ms++;
}

/* g_ms 기반의 정확한 밀리초 지연. 부호 없는 뺄셈이라 래핑도 안전. */
static void delay_ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) {
        /* 대기 */
    }
}

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));   /* 푸시풀 출력 2MHz */

    /* 1ms 틱: SystemCoreClock/1000 (예: 72MHz → 72000) */
    SysTick_Config(SystemCoreClock / 1000u);

    while (1) {
        GPIOC->ODR ^= (1u << LED_PIN);   /* PC13 토글 */
        delay_ms(500u);                  /* 정확히 500ms */
    }
}
```

### 예제 2 — `pc_test.c` : 래핑 안전한 경과 시간 검산
```c
#include <stdio.h>
#include <stdint.h>

/*
 * 16강 PC 검산 — 부호 없는 틱 카운터의 경과 시간 계산.
 * elapsed = now - start (uint32 래핑 산술). 카운터가 한 바퀴 돌아도
 * 빼기 결과가 올바른 경과 시간을 준다(실제 경과가 2^32 이내일 때).
 * 그래서 delay_ms 의 (g_ms - start) < ms 가 래핑에도 안전하다.
 */

static uint32_t elapsed(uint32_t now, uint32_t start)
{
    return now - start;     /* 부호 없는 뺄셈 → 래핑 자동 처리 */
}

int main(void)
{
    /* {start, now} 쌍: 마지막 둘은 카운터가 한 바퀴 돈(래핑) 경우 */
    uint32_t s[4] = { 1000u, 0u, 0xFFFFFFFFu, 0xFFFFFF00u };
    uint32_t n[4] = { 1500u, 10u, 0x00000004u, 0x00000100u };
    int i;

    for (i = 0; i < 4; i++) {
        printf("start=%lu now=%lu -> elapsed=%lu\n",
               (unsigned long)s[i], (unsigned long)n[i],
               (unsigned long)elapsed(n[i], s[i]));
    }
    return 0;
}
```

## 시뮬레이터로 확인하기
- `g_ms`를 Watch에 추가하고 실행하면, 1ms마다 값이 증가한다(시뮬 시간 기준).
- Logic Analyzer에 `PORTC.13`을 추가하면 정확히 500ms 간격(1초 주기)으로 토글한다.
- `SysTick->LOAD`를 레지스터 창에서 보면 `71999`(72000-1, 72MHz 가정).
- **예상 동작**: PC13이 1초 주기(500ms ON/500ms OFF)로 토글, `g_ms`는 꾸준히 증가.

## PC에서 검증하기

```
gcc pc_test.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
start=1000 now=1500 -> elapsed=500
start=0 now=10 -> elapsed=10
start=4294967295 now=4 -> elapsed=5
start=4294967040 now=256 -> elapsed=512
```

마지막 두 줄이 핵심입니다. 카운터가 최댓값 근처에서 0으로 넘어가도(래핑), 부호 없는 뺄셈이 올바른 경과(5, 512)를 줍니다.

## 자주 하는 실수

### Q. `g_ms`가 안 변하거나 최적화로 사라져요.
A. `volatile`을 빠뜨렸기 때문입니다. 인터럽트 핸들러와 메인이 공유하는 변수는 반드시 `volatile`로 선언하세요.

### Q. `SysTick_Handler` 이름을 다르게 지었더니 인터럽트가 안 걸려요.
A. 핸들러 이름은 startup 벡터 테이블과 **정확히** 일치해야 합니다. CMSIS 표준 이름은 `SysTick_Handler`입니다. 철자가 다르면 기본(빈) 핸들러가 호출됩니다.

### Q. 1초(1000ms) 주기를 SysTick 하나로 만들려는데 안 돼요.
A. 24비트 한계로 72MHz에서 한 주기는 최대 ~233ms입니다. 긴 시간은 1ms 틱을 세는 방식(이 편처럼 `g_ms`)으로 만드세요.

### Q. `SystemCoreClock` 값이 예상과 달라요.
A. `SystemCoreClock`은 `system_stm32f10x.c`의 `SystemInit()`이 설정한 값입니다. 클럭 설정을 바꾸면 `SystemCoreClockUpdate()`를 호출하거나 값을 갱신해야 정확합니다.

## 정리
- SysTick은 모든 Cortex-M에 내장된 24비트 다운 카운터로 이식성이 좋다.
- `SysTick_Config(SystemCoreClock/1000)`으로 1ms 주기 인터럽트를 만든다.
- 핸들러에서 `volatile` 밀리초 카운터를 증가시킨다.
- `delay_ms`는 부호 없는 뺄셈으로 래핑에도 안전하다.
- 24비트 한계(72MHz에서 ~233ms)는 틱 카운팅으로 넘는다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[17. 범용 타이머·PWM 출력](../17_TIM_PWM/README.md) — 코어 타이머를 넘어 범용 타이머(TIM)로 갑니다. PWM으로 LED 밝기를 부드럽게 조절하는 법을 배웁니다.
