# 과제 - 14. RCC·GPIO 레지스터로 LED

문제 1은 **순수 로직(PC `gcc`)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — F1 GPIO 4비트 설정 코드 계산
- 파일명: `homework_01/main.c`
- 핵심 개념: MODE/CNF 비트 조합, CRL/CRH 4비트
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- (MODE, CNF) 쌍으로부터 핀 설정 4비트 코드 `(CNF << 2) | MODE`를 계산해 출력한다.
- 아래 5가지 조합을 순서대로 출력한다. 형식 `mode=M cnf=C -> 0xN`은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
mode=0 cnf=1 -> 0x4
mode=2 cnf=0 -> 0x2
mode=3 cnf=0 -> 0x3
mode=2 cnf=1 -> 0x6
mode=0 cnf=2 -> 0x8
```

### 힌트
- 하위 2비트가 MODE, 그 위 2비트가 CNF다: `code = (cnf << 2) | mode;`.
- 첫 줄(0x4)은 입력 플로팅, 둘째(0x2)는 출력 2MHz 푸시풀이다.

---

## 문제 2 — PA5 출력 점멸
- 파일명: `homework_02/main.c`
- 핵심 개념: GPIO 클럭, CRL 설정, ODR 토글
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- GPIOA 클럭을 켜고, PA5를 푸시풀 출력(2MHz)으로 설정한다.
- `while(1)`에서 PA5를 토글하며 지연을 둔다.

### 예상 동작 (시뮬레이터)
- PA5는 핀0~7 영역이라 `CRL`을 쓴다(위치 `5*4=20`). 설정 후 `GPIOA->CRL`의 [23:20]이 `0x2`가 되어 값이 `0x44244444`가 된다.
- Logic Analyzer에 `PORTA.5`를 추가하면 주기적 토글 사각파가 보인다.

### 힌트
- `RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;`
- `GPIOA->CRL &= ~(0xFu << (5*4)); GPIOA->CRL |= (0x2u << (5*4));`
- 토글: `GPIOA->ODR ^= (1u << 5);`

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
