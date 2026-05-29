# 과제 - 18. NVIC 인터럽트와 EXTI

문제 1은 **순수 로직(PC `gcc`)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — EXTI 라인 → EXTICR 인덱스/시프트 계산
- 파일명: `homework_01/main.c`
- 핵심 개념: `EXTICR[n/4]`, 시프트 `(n%4)*4`
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- 핀 번호(EXTI 라인) 0, 3, 4, 7, 13에 대해 어느 `EXTICR` 배열 인덱스와 비트 시프트를 쓰는지 계산해 출력한다.
- 인덱스 = `n / 4`, 시프트 = `(n % 4) * 4`.
- 형식 `EXTIn -> EXTICR[i], shift s`는 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
EXTI0 -> EXTICR[0], shift 0
EXTI3 -> EXTICR[0], shift 12
EXTI4 -> EXTICR[1], shift 0
EXTI7 -> EXTICR[1], shift 12
EXTI13 -> EXTICR[3], shift 4
```

### 힌트
- `idx = n / 4; shift = (n % 4) * 4;`.
- EXTI13은 `13/4=3`, `(13%4)*4 = 1*4 = 4`.

---

## 문제 2 — EXTI1(PA1) 하강 에지 인터럽트
- 파일명: `homework_02/main.c`
- 핵심 개념: 하강 에지(FTSR), 풀업, EXTI1 라인
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- PA1을 입력 + 내부 **풀업**으로 두고(버튼은 PA1~GND, 누르면 하강 에지), EXTI1으로 하강 에지를 잡는다.
- 인터럽트마다 PC13 LED를 토글한다.

### 예상 동작 (시뮬레이터)
- Command 창에서 `PORTA &= ~0x0002`(PA1=0, 하강 에지)를 만들면 `EXTI1_IRQHandler`가 호출되어 PC13이 토글된다.
- EXTI의 FTSR 비트1=1, IMR 비트1=1, NVIC에서 EXTI1 Enabled.

### 힌트
- 매핑: `AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;` (EXTI1도 EXTICR[0], 0=PA).
- 트리거: `EXTI->FTSR |= EXTI_FTSR_TR1;` 마스크: `EXTI->IMR |= EXTI_IMR_MR1;`.
- `NVIC_EnableIRQ(EXTI1_IRQn);`, 핸들러는 `EXTI1_IRQHandler`, 클리어 `EXTI->PR = EXTI_PR_PR1;`.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
