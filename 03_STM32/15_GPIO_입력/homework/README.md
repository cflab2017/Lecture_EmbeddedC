# 과제 - 15. GPIO 입력·풀업/풀다운

문제 1은 **순수 로직(PC `gcc`)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — 버튼 누름(에지)에서 LED 토글
- 파일명: `homework_01/main.c`
- 핵심 개념: 상승 에지 검출, 상태 유지
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- 8개의 눌림 샘플 `{0,1,1,0,1,0,0,1}`을 차례로 처리한다(1=눌림).
- **새로 눌린 순간(상승 에지: 직전 0 → 현재 1)** 에만 LED를 토글한다. LED 시작은 OFF.
- 매 샘플마다 현재 눌림 상태와 LED 상태를 출력한다. 형식은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
sample 0: pressed=0 LED=OFF
sample 1: pressed=1 LED=ON
sample 2: pressed=1 LED=ON
sample 3: pressed=0 LED=ON
sample 4: pressed=1 LED=OFF
sample 5: pressed=0 LED=OFF
sample 6: pressed=0 LED=OFF
sample 7: pressed=1 LED=ON
```

### 힌트
- 직전 상태 `prev`를 기억하고, `pressed && !prev`일 때만 `led = !led`.
- 눌려 있는 동안(연속 1) 토글하면 안 된다 — 에지에서만.

---

## 문제 2 — 풀다운 입력(액티브 하이) 버튼
- 파일명: `homework_02/main.c`
- 핵심 개념: 풀다운 선택(ODR=0), 액티브 하이 판정
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- PA1을 입력 + 내부 **풀다운**으로 설정한다(버튼은 PA1과 VCC 사이, 눌림=1).
- 버튼이 눌리면(IDR 비트=1) PC13 LED를 켠다(액티브 로우 LED).

### 예상 동작 (시뮬레이터)
- `GPIOA->CRL`의 PA1 자리(비트[7:4])가 `0x8`이 되고, `ODR` 비트1=0(풀다운).
- Command 창에서 `PORTA |= 0x0002`(PA1=1, 눌림)이면 PC13=0(LED ON), `PORTA &= ~0x0002`이면 PC13=1(OFF).

### 힌트
- `GPIOA->ODR &= ~(1u << 1);` → 풀다운.
- 판정: `if (GPIOA->IDR & (1u << 1)) { /* 눌림 */ }`.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
