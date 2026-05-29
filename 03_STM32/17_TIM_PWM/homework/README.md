# 과제 - 17. 범용 타이머·PWM 출력

문제 1은 **순수 로직(PC `gcc`)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — 듀티(%)로부터 CCR 계산
- 파일명: `homework_01/main.c`
- 핵심 개념: CCR = duty × (ARR+1) / 100
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- ARR=999(주기 1000) 기준으로, 듀티 0·25·50·75·100%에 대한 CCR 값을 계산해 출력한다.
- `CCR = duty × (ARR+1) / 100`.
- 형식 `duty N% -> CCR R`은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
duty 0% -> CCR 0
duty 25% -> CCR 250
duty 50% -> CCR 500
duty 75% -> CCR 750
duty 100% -> CCR 1000
```

### 힌트
- `ccr = duty * (arr + 1u) / 100u;` (정수 연산 순서 주의 — 곱한 뒤 나눈다).
- ARR=999일 때 CCR=1000은 듀티 100%(항상 High)를 뜻한다.

---

## 문제 2 — TIM2 CH2(PA1) PWM 1kHz 75%
- 파일명: `homework_02/main.c`
- 핵심 개념: 다른 채널 설정(CCMR1 상위 바이트, CCER CC2E, CCR2)
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- TIM2 채널2(PA1)로 1kHz, 듀티 75% PWM을 출력한다.
- PSC=71, ARR=999, CCR2=750으로 설정한다.

### 예상 동작 (시뮬레이터)
- Logic Analyzer `PORTA.1`에 1kHz, 75% 듀티(High 0.75ms) 사각파.
- TIM2 레지스터에서 CCR2=750, CCER의 CC2E=1, CR1의 CEN=1 확인.

### 힌트
- PA1을 AF 푸시풀(0xB)로: `GPIOA->CRL` 비트[7:4].
- 채널2는 `CCMR1`의 상위 바이트: `TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1`, `TIM_CCMR1_OC2PE`.
- 출력 인에이블: `TIM_CCER_CC2E`. 듀티: `TIM2->CCR2 = 750;`.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
