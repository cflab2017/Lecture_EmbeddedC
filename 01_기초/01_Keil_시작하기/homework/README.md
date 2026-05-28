# 과제 - 01. 임베디드 C와 Keil μVision 시작하기

이번 과제는 모두 **순수 로직(PC 검증 가능)** 입니다. 보드나 μVision 없이 `gcc`만으로 풀고 채점할 수 있습니다.

```
gcc main.c -o prog && ./prog
```

---

## 문제 1 — 블링크 횟수와 ON 카운트
- 파일명: `homework_01/main.c`
- 핵심 개념: `while(1)` 골격, 토글, 카운팅
- 플랫폼: 공통

### 요구사항
- LED를 8번 토글(`tick` 0~7)하면서 매 tick의 상태를 출력한다. (시작 상태는 꺼짐 0)
- 8번 토글이 끝난 뒤, LED가 **ON이었던 횟수**를 마지막 줄에 출력한다.
- 출력 형식은 아래 "예상 출력"과 글자 단위로 일치해야 한다.

### 예상 출력
```
tick 0: LED = ON
tick 1: LED = OFF
tick 2: LED = ON
tick 3: LED = OFF
tick 4: LED = ON
tick 5: LED = OFF
tick 6: LED = ON
tick 7: LED = OFF
ON count = 4
```

### 힌트
- 시작 상태 0에서 토글하면 첫 tick은 ON이 된다.
- LED가 ON일 때마다 별도 카운터 변수를 1 증가시킨다.
- 8회 중 ON은 짝수 tick(0,2,4,6)에서 나오므로 총 4회.

---

## 문제 2 — 두 개의 LED 교대 점멸
- 파일명: `homework_02/main.c`
- 핵심 개념: 상태 변수, 상호 배타 출력
- 플랫폼: 공통

### 요구사항
- LED 두 개(A, B)를 다룬다. A가 ON이면 B는 OFF, A가 OFF면 B는 ON으로 항상 반대 상태를 유지한다.
- 시작 상태는 A=ON, B=OFF.
- 6 tick 동안 매 tick의 두 상태를 출력하고, 매 tick이 끝날 때 A를 토글한다(B는 항상 A의 반대).

### 예상 출력
```
tick 0: A = ON , B = OFF
tick 1: A = OFF, B = ON
tick 2: A = ON , B = OFF
tick 3: A = OFF, B = ON
tick 4: A = ON , B = OFF
tick 5: A = OFF, B = ON
```

### 힌트
- B는 따로 관리하지 말고 `b = !a;`처럼 A로부터 계산하면 항상 반대가 보장된다.
- 출력 정렬을 맞추려면 `ON `(뒤 공백 1칸)과 `OFF`처럼 폭을 맞춰 `%-3s`를 쓰거나 직접 분기한다.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
