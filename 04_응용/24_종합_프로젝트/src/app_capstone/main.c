#include "stm32f10x.h"
#include "ringbuffer.h"
#include "command.h"

/*
 * 24강 캡스톤 — UART 명령으로 LED·타이머 제어.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * 전체 흐름(앞 강의들의 종합):
 *   USART1 RX 인터럽트(19/18) → 링버퍼에 push(22)
 *   메인 루프: pop → 줄 조립(22) → 명령 파싱(command.c) → 동작
 *   SysTick(16): blink 모드에서 500ms 마다 LED 토글
 *   LED 제어(14): PC13 액티브 로우
 *
 * 명령: on / off / blink / help  (개행으로 한 줄 종료)
 */

#define LED_PIN    13u
#define BAUD_BRR   7500u      /* 72MHz / 9600 */
#define LINE_MAX   32u

static ringbuffer_t      rx_rb;
static volatile uint32_t g_ms = 0;
static volatile uint8_t  blink_mode = 0;

/* ---- 인터럽트 핸들러 ---- */
void SysTick_Handler(void)
{
    g_ms++;
    if (blink_mode && ((g_ms % 500u) == 0u)) {
        GPIOC->ODR ^= (1u << LED_PIN);     /* blink 모드: 500ms 토글 */
    }
}

void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t b = (uint8_t)USART1->DR;   /* 읽으면 RXNE 클리어 */
        (void)rb_push(&rx_rb, b);          /* ISR 은 넣기만 — 짧게 */
    }
}

/* ---- 하드웨어 초기화 ---- */
static void led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFu << ((LED_PIN - 8u) * 4u));
    GPIOC->CRH |=  (0x2u << ((LED_PIN - 8u) * 4u));
}

static void usart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->CRH |=  (0xBu << ((9u - 8u) * 4u));    /* PA9  TX AF 푸시풀 */
    GPIOA->CRH |=  (0x4u << ((10u - 8u) * 4u));   /* PA10 RX 플로팅 입력 */

    USART1->BRR = BAUD_BRR;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;
    NVIC_EnableIRQ(USART1_IRQn);
}

/* ---- UART 송신 헬퍼 ---- */
static void send_char(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = (uint16_t)c;
}

static void send_str(const char *s)
{
    while (*s) {
        send_char(*s++);
    }
}

/* ---- LED 동작 ---- */
static void led_on(void)  { GPIOC->BRR  = (1u << LED_PIN); }   /* 액티브 로우 ON */
static void led_off(void) { GPIOC->BSRR = (1u << LED_PIN); }   /* OFF */

/* ---- 명령 실행 ---- */
static void handle_command(command_t cmd)
{
    switch (cmd) {
        case CMD_ON:    blink_mode = 0u; led_on();  send_str("LED ON\r\n");          break;
        case CMD_OFF:   blink_mode = 0u; led_off(); send_str("LED OFF\r\n");         break;
        case CMD_BLINK: blink_mode = 1u;            send_str("BLINK mode\r\n");      break;
        case CMD_HELP:  send_str("cmds: on off blink help\r\n");                     break;
        case CMD_NONE:  /* 빈 줄 무시 */                                             break;
        default:        send_str("unknown command\r\n");                            break;
    }
}

int main(void)
{
    char     line[LINE_MAX];
    uint16_t idx = 0u;
    uint8_t  c;

    rb_init(&rx_rb);
    led_init();
    usart1_init();
    SysTick_Config(SystemCoreClock / 1000u);   /* 1ms 틱 */

    send_str("Capstone ready. Type: on/off/blink/help\r\n");

    while (1) {
        if (rb_pop(&rx_rb, &c)) {
            if (c == '\n' || c == '\r') {       /* 줄 끝 → 명령 처리 */
                if (idx > 0u) {
                    line[idx] = '\0';
                    handle_command(command_parse(line));
                    idx = 0u;
                }
            } else if (idx < (LINE_MAX - 1u)) {
                line[idx++] = (char)c;          /* 글자 누적 */
            }
        }
    }
}
