# 과제 - 16. SysTick 타이머와 정확한 지연

문제 1은 **순수 로직(PC `gcc`)**, 문제 2는 **MDK(STM32F103C8, μVision 시뮬레이터)** 입니다.

---

## 문제 1 — SysTick reload 계산과 24비트 한계
- 파일명: `homework_01/main.c`
- 핵심 개념: reload = ticks − 1, 24비트 한계(0xFFFFFF)
- 플랫폼: 공통 (PC `gcc`)

### 요구사항
- 코어 클럭 72MHz를 가정하고, 주기 1·10·100·1000ms 각각에 대해 SysTick reload 값을 계산한다.
- `ticks = 72000 × period_ms`, `reload = ticks − 1`.
- reload가 24비트 한계 `0xFFFFFF`(16777215) 이하면 `(OK)`, 초과면 `(OVERFLOW)`를 붙인다.
- 형식 `period N ms -> reload R (...)`은 예상 출력과 글자 단위로 일치해야 한다.

### 예상 출력
```
period 1 ms -> reload 71999 (OK)
period 10 ms -> reload 719999 (OK)
period 100 ms -> reload 7199999 (OK)
period 1000 ms -> reload 71999999 (OVERFLOW)
```

### 힌트
- `uint32_t ticks = 72000u * period_ms; uint32_t reload = ticks - 1u;`
- 한계 비교: `reload <= 0xFFFFFFu ? "OK" : "OVERFLOW"`.
- 1000ms는 reload가 약 7200만이라 24비트(약 1677만)를 훌쩍 넘는다.

---

## 문제 2 — 1초마다 토글 + 초 카운터
- 파일명: `homework_02/main.c`
- 핵심 개념: SysTick 1ms 틱, delay_ms로 긴 주기
- 플랫폼: MDK (STM32F103C8)

### 요구사항
- SysTick으로 1ms 틱(`g_ms`)을 만든다.
- `delay_ms(1000)`으로 1초마다 PC13을 토글하고, `seconds` 변수를 1씩 증가시킨다.

### 예상 동작 (시뮬레이터)
- `g_ms`는 1ms마다 증가, `seconds`는 1초마다 1씩 증가(Watch로 확인).
- PC13(PORTC.13)은 1초마다 토글(2초 주기).

### 힌트
- 예제의 `SysTick_Handler` / `delay_ms` 패턴을 그대로 쓴다.
- `seconds`도 `volatile`로 두면 디버거에서 관찰이 안정적이다.

---

## 정답 확인
직접 풀어 본 후 [`answer/`](./answer/) 폴더의 정답과 비교해 보세요. 정답 파일에는 핵심 포인트와 흔한 실수까지 주석으로 정리되어 있습니다.
