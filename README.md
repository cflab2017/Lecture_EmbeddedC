# Lecture_EmbeddedC — 임베디드 C 입문 (Keil μVision)

운영체제도, 화면도 없는 작은 칩 위에서 C로 하드웨어를 직접 다루는 법을 배우는 **한국어 임베디드 C 강의 24편**입니다. **Keil μVision**(8051: Keil C51, STM32: Keil MDK-ARM / Arm Compiler 6)을 기준으로 하고, 검증은 **μVision 내장 시뮬레이터**(실제 보드 불필요)로 합니다. STM32 HAL/SPL 라이브러리 없이 **레지스터를 직접 제어**해 학습 효과를 높이는 것이 이 트랙의 방침입니다.

하드웨어에 의존하지 않는 순수 로직(비트 유틸·디바운스·링버퍼 등)은 PC의 `gcc`로 빌드해 `printf`로 동작을 확인할 수 있는 **PC 테스트 하니스(`pc_test.c`)** 를 함께 제공합니다.

## 이런 분께
- C 문법은 어느 정도 알지만 임베디드(MCU)는 처음인 분
- 레지스터·데이터시트를 직접 다루며 "밑바닥부터" 이해하고 싶은 분
- 보드가 없어도 시뮬레이터·PC로 끝까지 따라오고 싶은 분

## 대상 환경 · 툴체인
| 플랫폼 | 컴파일러 | 디바이스 | 다운로드 |
|--------|----------|----------|----------|
| 공통(순수 로직) | PC `gcc`(또는 μVision) | 없음 | 시스템 gcc |
| 8051 | **Keil C51** | AT89C52 | https://www.keil.com/demo/eval/c51.htm |
| STM32 | **Keil MDK-ARM (Arm Compiler 6)** | STM32F103C8 | https://www.keil.com/demo/eval/arm.htm |
| (확장) 아두이노 우노 | avr-gcc / Arduino IDE | ATmega328P | https://www.arduino.cc/en/software |

- 전체 제품: https://www.keil.com/download/product/ · MDK 커뮤니티(무료): https://www.keil.arm.com/mdk-community/
- 설치/프로젝트 생성 절차는 [1편](01_기초/01_Keil_시작하기/README.md)(공통)·[6편](02_8051/06_8051_구조와_SFR/README.md)(8051)·[14편](03_STM32/14_RCC_GPIO_LED/README.md)(STM32)에 단계별로 있습니다.

## 커리큘럼 (24편)

### 1부 · 기초 (공통 — PC `gcc`로 검증)
| # | 강의 | 핵심 |
|---|------|------|
| 01 | [임베디드 C와 Keil μVision 시작하기](01_기초/01_Keil_시작하기/README.md) | 임베디드 C, `while(1)` 골격, 빌드/디버그 흐름 |
| 02 | [자료형·진수·stdint·sizeof](01_기초/02_자료형과_진수/README.md) | 정확한 비트 폭, `uint8/16/32_t` |
| 03 | [비트 연산과 레지스터 조작](01_기초/03_비트_연산/README.md) | set/clear/toggle, 마스크 |
| 04 | [포인터·volatile·메모리 맵드 I/O](01_기초/04_포인터와_메모리맵/README.md) | 레지스터 = 주소, `volatile` |
| 05 | [함수·헤더 분리·#define 매크로](01_기초/05_함수와_전처리기/README.md) | 모듈화, 헤더 가드 |

### 2부 · 8051 (Keil C51 / AT89C52)
| # | 강의 | 핵심 |
|---|------|------|
| 06 | [8051 구조·SFR·reg52.h](02_8051/06_8051_구조와_SFR/README.md) | SFR, `sbit`/`sfr` |
| 07 | [GPIO 출력 — LED 점멸](02_8051/07_GPIO_출력_LED/README.md) | 포트 출력, 액티브 로우 |
| 08 | [GPIO 입력 — 버튼/스위치](02_8051/08_GPIO_입력_버튼/README.md) | 준쌍방향 포트 입력 |
| 09 | [타이머0/1 과 지연](02_8051/09_타이머/README.md) | 하드웨어 타이머 |
| 10 | [외부·타이머 인터럽트](02_8051/10_인터럽트/README.md) | `interrupt N` |
| 11 | [UART 시리얼 통신](02_8051/11_UART/README.md) | 직렬 송수신 |
| 12 | [7세그먼트 표시 (멀티플렉싱)](02_8051/12_7세그먼트/README.md) | 룩업 테이블, 동적 구동 |

### 3부 · STM32 (Keil MDK-ARM / STM32F103C8, 레지스터 직접 제어)
| # | 강의 | 핵심 |
|---|------|------|
| 13 | [Cortex-M·CMSIS·클럭 트리](03_STM32/13_STM32_구조와_CMSIS/README.md) | CMSIS, RCC, 클럭 트리 |
| 14 | [RCC·GPIO 레지스터로 LED](03_STM32/14_RCC_GPIO_LED/README.md) | CRL/CRH, ODR/BSRR |
| 15 | [GPIO 입력·풀업/풀다운](03_STM32/15_GPIO_입력/README.md) | IDR, 내부 풀업 |
| 16 | [SysTick 타이머와 정확한 지연](03_STM32/16_SysTick/README.md) | `SysTick_Config`, ms 틱 |
| 17 | [범용 타이머·PWM 출력](03_STM32/17_TIM_PWM/README.md) | PSC/ARR/CCR, PWM |
| 18 | [NVIC 인터럽트와 EXTI](03_STM32/18_NVIC_EXTI/README.md) | EXTI, 펜딩 클리어 |
| 19 | [USART 시리얼 통신](03_STM32/19_USART/README.md) | BRR, TXE/RXNE |
| 20 | [ADC 아날로그 입력](03_STM32/20_ADC/README.md) | 12비트 변환, 환산 |

### 4부 · 응용·패턴
| # | 강의 | 플랫폼 |
|---|------|--------|
| 21 | [디바운싱·유한 상태 머신 패턴](04_응용/21_디바운싱_상태머신/README.md) | 공통 |
| 22 | [링버퍼와 인터럽트 기반 UART 수신](04_응용/22_링버퍼_인터럽트_UART/README.md) | 공통 |
| 23 | [HAL 추상화·모듈 분리·이식성](04_응용/23_펌웨어_구조화/README.md) | 공통 (+아두이노 우노/AVR 확장) |
| 24 | [종합 — UART 명령으로 LED·타이머 제어](04_응용/24_종합_프로젝트/README.md) | MDK (캡스톤) |

## 폴더 구조 (한 편당)
```
<강의 폴더>/
├── README.md                  강의 본문
├── src/<project_name>/
│   ├── main.c                 디바이스용 예제(레지스터 제어)
│   ├── <module>.c/.h          모듈 분리 시
│   └── pc_test.c              (순수 로직) gcc 검증 하니스
└── homework/
    ├── README.md              과제 문제지
    └── answer/                정답(주석 포함)
```

## 빌드 · 검증
- **순수 로직(공통)**: `gcc pc_test.c -o pc_test && ./pc_test` — 각 README의 "예상 출력"과 대조. (모듈 편은 `gcc pc_test.c <module>.c -o pc_test`)
- **8051/STM32(디바이스)**: 각 README의 "μVision 프로젝트 만들기" 절차대로 프로젝트 생성 → Rebuild(F7) → Debug(Ctrl+F5) → Logic Analyzer / UART #1 / 레지스터 창으로 "예상 동작" 확인.
- **아두이노 우노(23편 확장)**: `avr-gcc -mmcu=atmega328p ...`로 빌드 (Keil은 AVR 미지원).

### 검증 상태
전체 소스를 실제 Keil 툴체인으로 컴파일 확인했습니다.

| 분류 | 컴파일러 | 결과 |
|------|----------|------|
| C51 (8051) | C51 9.59 | 0 warning / 0 error |
| 공통 순수 로직 | Arm Compiler 6 (`armclang -Wall -Wextra`) | 무경고 |
| STM32 레지스터 코드 | Arm Compiler 6 + CMSIS/STM32F1 헤더 | 무경고 |
| 아두이노 우노 HAL | avr-gcc 7.3.0 (atmega328p) | 빌드/링크 통과 |

> 순수 로직의 `printf` 출력은 결정적(deterministic)이며, 각 README의 "예상 출력"과 수기 트레이스로 대조했습니다.

## 라이선스
학습용 예제 코드입니다. 자유롭게 학습·수정에 사용하세요.
