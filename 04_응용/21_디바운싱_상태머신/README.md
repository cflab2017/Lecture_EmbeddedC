# 21. 디바운싱·유한 상태 머신 패턴

주변장치 사용법을 익혔으니, 이제 **펌웨어를 짜는 패턴**으로 넘어갑니다. 그 첫 번째가 거의 모든 임베디드 프로젝트에 등장하는 두 가지 — **디바운싱**과 **유한 상태 머신(FSM)** 입니다. 기계식 버튼은 누르고 떼는 순간 접점이 수 ms 동안 튀어(채터링), 한 번 눌렀는데 여러 번 눌린 것처럼 읽힙니다. 디바운싱은 이 떨림을 걸러 "진짜" 입력만 남깁니다. FSM은 "지금 상태 + 입력 → 다음 상태"라는 단순한 규칙으로 복잡한 동작을 깔끔하게 표현하는 설계 도구입니다.

이번 편은 하드웨어에 의존하지 않는 **순수 로직**입니다. 디바운서를 재사용 가능한 모듈(`debounce.c/.h`)로 분리하고, 그 위에 LED 모드 FSM을 얹어, PC의 `gcc`로 동작을 정확히 검증합니다. 여기서 만든 디바운서는 8051이든 STM32든 그대로 가져다 쓸 수 있습니다.

## 학습 목표
- 버튼 채터링(바운스)의 원인과 증상을 설명한다.
- 적분기(연속 샘플 카운트) 방식의 디바운서를 구현한다.
- 디바운서를 모듈로 분리해 재사용·테스트 가능하게 만든다.
- 유한 상태 머신의 구성요소(상태·입력·전이)를 이해한다.
- 디바운스된 "확정 에지"로 FSM을 구동한다.

## 대상 환경
- 컴파일러: **PC + gcc** 또는 μVision(순수 로직이라 어디서나 동작).
- 디바이스: 없음(하드웨어 비의존). 실제 보드에선 raw 샘플이 GPIO `IDR`에서 온다.
- 검증: PC `gcc`(`pc_test.c`)로 예상 출력 확인.

## 핵심 개념

### 1) 채터링(바운스)
기계식 버튼의 금속 접점은 닿는 순간 미세하게 튕깁니다. 그래서 누르는 한 번의 동작이 전기적으로는 `0→1→0→1→…→1`처럼 수 ms간 진동합니다. 이 신호를 그대로 읽으면 한 번 누름이 여러 번으로 카운트됩니다.

### 2) 디바운싱 전략
| 방식 | 아이디어 | 특징 |
|------|----------|------|
| 시간 지연 | 변화 감지 후 수십 ms 무시 | 단순하지만 블로킹·놓침 위험 |
| 적분기(샘플 카운트) | 일정 주기로 샘플, 같은 값이 N번 연속이면 확정 | 논블로킹·견고(이 편 채택) |

### 3) 적분기 디바운서
일정 주기(예: SysTick 1~5ms)로 핀을 샘플링하고, **안정 상태와 다른 값이 THRESHOLD번 연속**될 때만 상태를 바꿉니다. 중간에 한 번이라도 원래 값으로 돌아오면 카운터가 0으로 리셋되어 글리치가 걸러집니다.

```c
uint8_t debounce_update(debounce_t *db, uint8_t raw)
{
    uint8_t r = raw ? 1u : 0u;
    if (r == db->stable) { db->count = 0u; return 0u; }   /* 떨림 없음 */
    db->count++;
    if (db->count >= DEBOUNCE_THRESHOLD) {                /* N번 연속 */
        db->stable = r; db->count = 0u; return 1u;        /* 확정 변경 */
    }
    return 0u;
}
```

### 4) 유한 상태 머신(FSM)
FSM은 **유한한 상태들**과, **입력에 따라 상태를 옮기는 전이 규칙**으로 동작을 표현합니다. "버튼을 누를 때마다 LED 모드가 OFF→SLOW→FAST→OFF로 순환"을 FSM으로 보면:

| 현재 상태 | 입력(눌림 확정) | 다음 상태 |
|-----------|-----------------|-----------|
| OFF | press | SLOW |
| SLOW | press | FAST |
| FAST | press | OFF |

C에서는 보통 `enum` + `switch`로 구현합니다. 상태가 명시적이라 디버깅과 확장이 쉽습니다.

### 5) 디바운스 + FSM 결합
FSM은 **디바운서가 확정한 "눌림 에지"** 만 입력으로 받습니다. `debounce_update`가 1을 돌려주고 새 안정 상태가 1(눌림)이면, 그때만 모드를 전진시킵니다. 이렇게 분리하면 입력 정제(디바운스)와 동작 결정(FSM)이 깔끔하게 나뉩니다.

### 6) 실제 하드웨어 연결
보드에서는 SysTick 같은 주기 인터럽트에서 `raw = (GPIOA->IDR >> pin) & 1`을 읽어 `debounce_update`에 넣습니다. 즉 이 편의 로직은 그대로 두고, raw 공급원만 GPIO로 바뀝니다.

## 예제로 보기

### 예제 1 — `debounce.h` / `debounce.c` : 재사용 디바운서 모듈
```c
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdint.h>

/*
 * 디바운서(적분기 방식).
 * 일정 주기로 raw 핀 값을 넣으면, 안정 상태와 다른 입력이 THRESHOLD 번
 * 연속될 때만 상태를 바꾼다. 짧은 글리치(채터링)는 무시된다.
 * 하드웨어 비의존이라 PC(gcc)에서 그대로 테스트할 수 있다.
 */

/* 새 입력이 이만큼 연속돼야 상태를 확정 변경한다. */
#define DEBOUNCE_THRESHOLD  3u

typedef struct {
    uint8_t stable;   /* 확정된 안정 상태 (0 또는 1) */
    uint8_t count;    /* 안정 상태와 다른 입력이 연속된 횟수 */
} debounce_t;

/* 디바운서를 초기 상태(0 또는 1)로 리셋한다. */
void debounce_init(debounce_t *db, uint8_t init);

/* raw 샘플 하나를 처리한다. 안정 상태가 바뀌면 1, 아니면 0 을 돌려준다. */
uint8_t debounce_update(debounce_t *db, uint8_t raw);

/* 현재 확정된 안정 상태(0/1). */
uint8_t debounce_state(const debounce_t *db);

#endif /* DEBOUNCE_H */
```

```c
#include "debounce.h"

void debounce_init(debounce_t *db, uint8_t init)
{
    db->stable = init ? 1u : 0u;
    db->count  = 0u;
}

uint8_t debounce_update(debounce_t *db, uint8_t raw)
{
    uint8_t r = raw ? 1u : 0u;

    if (r == db->stable) {
        db->count = 0u;            /* 흔들림 없음 → 카운터 리셋 */
        return 0u;
    }

    db->count++;                   /* 안정 상태와 다른 입력 누적 */
    if (db->count >= DEBOUNCE_THRESHOLD) {
        db->stable = r;            /* 충분히 연속 → 상태 확정 변경 */
        db->count  = 0u;
        return 1u;                 /* 변경됨 */
    }
    return 0u;
}

uint8_t debounce_state(const debounce_t *db)
{
    return db->stable;
}
```

### 예제 2 — `pc_test.c` : 디바운서 + 모드 FSM
```c
#include <stdio.h>
#include <stdint.h>
#include "debounce.h"

/*
 * 21강 PC 검산 — 디바운서 + LED 모드 FSM.
 * 노이즈가 낀 raw 버튼 스트림을 디바운스하고, '눌림으로 확정'되는 에지마다
 * 모드를 OFF→SLOW→FAST→OFF 로 순환시킨다.
 * 빌드: gcc pc_test.c debounce.c -o pc_test
 */

typedef enum { MODE_OFF = 0, MODE_SLOW = 1, MODE_FAST = 2 } led_mode_t;

static const char *mode_name(led_mode_t m)
{
    switch (m) {
        case MODE_OFF:  return "OFF";
        case MODE_SLOW: return "SLOW";
        case MODE_FAST: return "FAST";
        default:        return "?";
    }
}

int main(void)
{
    /* 1=눌림. 글리치(인덱스2)와 짧은 떨림(인덱스13)이 섞여 있다. */
    uint8_t raw[20] = {
        0,0,1,0,1,1,1,0,0,0,
        1,1,1,0,1,1,1,1,0,0
    };
    debounce_t db;
    led_mode_t mode = MODE_OFF;
    int i;

    debounce_init(&db, 0u);

    for (i = 0; i < 20; i++) {
        uint8_t changed = debounce_update(&db, raw[i]);
        if (changed && debounce_state(&db) == 1u) {
            mode = (led_mode_t)((mode + 1) % 3);   /* 눌림 확정 에지에서만 전진 */
        }
        printf("t=%2d raw=%u stable=%u mode=%s\n",
               i, raw[i], debounce_state(&db), mode_name(mode));
    }
    return 0;
}
```

## PC에서 검증하기

```
gcc pc_test.c debounce.c -o pc_test && ./pc_test
```

예상 출력(stdout):

```
t= 0 raw=0 stable=0 mode=OFF
t= 1 raw=0 stable=0 mode=OFF
t= 2 raw=1 stable=0 mode=OFF
t= 3 raw=0 stable=0 mode=OFF
t= 4 raw=1 stable=0 mode=OFF
t= 5 raw=1 stable=0 mode=OFF
t= 6 raw=1 stable=1 mode=SLOW
t= 7 raw=0 stable=1 mode=SLOW
t= 8 raw=0 stable=1 mode=SLOW
t= 9 raw=0 stable=0 mode=SLOW
t=10 raw=1 stable=0 mode=SLOW
t=11 raw=1 stable=0 mode=SLOW
t=12 raw=1 stable=1 mode=FAST
t=13 raw=0 stable=1 mode=FAST
t=14 raw=1 stable=1 mode=FAST
t=15 raw=1 stable=1 mode=FAST
t=16 raw=1 stable=1 mode=FAST
t=17 raw=1 stable=1 mode=FAST
t=18 raw=0 stable=1 mode=FAST
t=19 raw=0 stable=1 mode=FAST
```

관찰 포인트: 인덱스2의 글리치는 무시되고(stable 유지), 인덱스4~6의 진짜 누름에서만 SLOW로 바뀝니다. 인덱스13의 짧은 떨림도 무시되어, 세 번째 "누름"은 stable이 이미 1이라 모드를 바꾸지 않습니다.

## 자주 하는 실수

### Q. 디바운스했는데도 가끔 두 번 눌린 것처럼 동작해요.
A. THRESHOLD가 너무 작거나 샘플 주기가 너무 빠릅니다. 보통 샘플 주기 × THRESHOLD가 10~30ms가 되도록 잡습니다(예: 5ms × 3 = 15ms).

### Q. 버튼을 누르고 있는 동안 모드가 계속 바뀌어요.
A. "눌림 상태"가 아니라 "**눌림으로 바뀐 에지**"에서만 동작해야 합니다. `debounce_update`의 반환값(변경됨)과 새 상태가 1인지를 함께 확인하세요.

### Q. FSM을 if-else 더미로 짰더니 상태가 꼬여요.
A. 상태를 `enum`으로 명시하고 `switch`로 전이를 한곳에 모으세요. 상태가 코드 곳곳에 흩어지면 추적이 어렵습니다.

### Q. 디바운스 로직을 인터럽트 안에 다 넣어도 되나요?
A. 주기 샘플링(틱)에서 `debounce_update`만 호출하고, 무거운 동작(FSM 결과 처리)은 메인 루프에서 하는 분리가 안전합니다.

## 정리
- 채터링은 기계 접점의 떨림으로, 한 번 누름이 여러 번으로 읽힌다.
- 적분기 디바운서는 같은 값이 N번 연속될 때만 상태를 확정해 글리치를 거른다.
- 디바운서를 모듈로 분리하면 어느 칩에서도 재사용·테스트할 수 있다.
- FSM은 상태·입력·전이로 동작을 표현하며 `enum`+`switch`로 구현한다.
- 입력 정제(디바운스)와 동작 결정(FSM)을 분리하면 코드가 깔끔하다.

## 직접 해 보기
`homework/` 폴더의 과제를 풀어 보세요. 정답은 `homework/answer/`에 있습니다.

## 다음 단원
[22. 링버퍼와 인터럽트 기반 UART 수신](../22_링버퍼_인터럽트_UART/README.md) — 인터럽트가 받은 데이터를 잃지 않고 메인으로 넘기는 핵심 자료구조, 링버퍼를 순수 로직으로 구현합니다.
