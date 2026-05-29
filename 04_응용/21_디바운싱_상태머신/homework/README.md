# 과제 - 21. 디바운싱·유한 상태 머신 패턴

두 문제 모두 **순수 로직(PC `gcc`)** 이라 보드 없이 채점할 수 있습니다.

```
gcc main.c -o prog && ./prog
```

---

## 문제 1 — 디바운서로 PRESS/RELEASE 이벤트 검출
- 파일명: `homework_01/main.c`
- 핵심 개념: 적분기 디바운스(THRESHOLD=2), 양방향 에지
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- THRESHOLD를 **2**로 둔 디바운서로 raw 스트림 `{0,1,1,0,0,1,1,1,0,0,1,1}`을 처리한다.
- 안정 상태가 0→1로 바뀌면 `PRESS`, 1→0으로 바뀌면 `RELEASE`를 그 시각과 함께 출력한다.
- 마지막에 총 PRESS 횟수를 출력한다. 형식은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
t=2 PRESS
t=4 RELEASE
t=6 PRESS
t=9 RELEASE
t=11 PRESS
total presses = 3
```

### 힌트
- `stable`과 다른 입력이 2번 연속이면 상태를 뒤집는다. 같은 값이 오면 카운터 리셋.
- 새 안정 상태가 1이면 PRESS, 0이면 RELEASE. PRESS일 때만 카운트 증가.

---

## 문제 2 — 4단계 모드 FSM
- 파일명: `homework_02/main.c`
- 핵심 개념: enum 상태, 순환 전이
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- 상태 `{OFF, LOW, MID, HIGH}`(4개)를 누름 이벤트마다 순환시킨다(HIGH 다음은 OFF).
- 입력은 "이번 틱에 눌림 확정이 있었는가" 배열 `{1,0,1,1,0,1,0,1}`(1=눌림).
- 매 틱의 입력과 현재 상태를 출력한다. 형식은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
t=0 press=1 state=LOW
t=1 press=0 state=LOW
t=2 press=1 state=MID
t=3 press=1 state=HIGH
t=4 press=0 state=HIGH
t=5 press=1 state=OFF
t=6 press=0 state=OFF
t=7 press=1 state=LOW
```

### 힌트
- 상태 전진: `state = (state + 1) % 4;` (press가 1일 때만).
- 상태 이름은 `switch`나 이름 배열로 출력.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
