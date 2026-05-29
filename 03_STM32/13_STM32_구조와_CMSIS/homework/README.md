# 과제 - 13. Cortex-M·CMSIS·클럭 트리

문제 1은 **순수 로직(PC `gcc` 검증)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — PLL 배수별 SYSCLK와 한계 검사
- 파일명: `homework_01/main.c`
- 핵심 개념: 클럭 트리, PLL 체배, F103의 72MHz 한계
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- HSE = 8MHz를 가정하고, PLL 배수 `mul`을 2부터 16까지 돌며 SYSCLK(`8MHz × mul`)를 출력한다.
- 각 줄 끝에 SYSCLK가 72MHz 이하이면 `(OK)`, 초과하면 `(OVER)`를 붙인다.
- 출력 형식 `PLLMUL xN -> ... Hz (...)`은 예상 출력과 글자 단위로 일치해야 한다. (배수는 `%-2u`로 왼쪽 정렬)

### 예상 출력
```
PLLMUL x2  -> 16000000 Hz (OK)
PLLMUL x3  -> 24000000 Hz (OK)
PLLMUL x4  -> 32000000 Hz (OK)
PLLMUL x5  -> 40000000 Hz (OK)
PLLMUL x6  -> 48000000 Hz (OK)
PLLMUL x7  -> 56000000 Hz (OK)
PLLMUL x8  -> 64000000 Hz (OK)
PLLMUL x9  -> 72000000 Hz (OK)
PLLMUL x10 -> 80000000 Hz (OVER)
PLLMUL x11 -> 88000000 Hz (OVER)
PLLMUL x12 -> 96000000 Hz (OVER)
PLLMUL x13 -> 104000000 Hz (OVER)
PLLMUL x14 -> 112000000 Hz (OVER)
PLLMUL x15 -> 120000000 Hz (OVER)
PLLMUL x16 -> 128000000 Hz (OVER)
```

### 힌트
- `sysclk = 8000000u * mul;` — `unsigned`로 충분(128MHz < 2^32).
- 한계 비교: `sysclk <= 72000000u ? "OK" : "OVER"`.
- 출력: `printf("PLLMUL x%-2u -> %lu Hz (%s)\n", mul, (unsigned long)sysclk, tag);`.

---

## 문제 2 — GPIOB와 TIM2 클럭 켜기
- 파일명: `homework_02/main.c`
- 핵심 개념: RCC 클럭 게이팅, APB1 vs APB2
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- GPIOB(APB2 버스)와 TIM2(APB1 버스)의 클럭을 각각 켠다.
- 두 주변장치가 서로 다른 인에이블 레지스터에 있다는 점을 코드로 드러낸다.
- `while(1)`로 마무리한다(STM32 main은 반환하지 않음).

### 예상 동작 (시뮬레이터)
- 디버그 중 RCC 레지스터를 보면 `APB2ENR`의 **IOPBEN(비트3)** 이 1(값 `0x08`), `APB1ENR`의 **TIM2EN(비트0)** 이 1(값 `0x01`)이 된다.
- 한 줄씩 실행(F11)하며 각 `|=` 직후 해당 비트가 세팅되는 것을 확인.

### 힌트
- `RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;` (GPIOB는 고속 APB2).
- `RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;` (TIM2는 저속 APB1).
- 비트 이름은 `stm32f10x.h`에 정의돼 있으니 매직 넘버를 쓰지 말 것.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
