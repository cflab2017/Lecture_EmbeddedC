#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — UART로 숫자를 10진수로 출력
 *
 * 핵심 포인트:
 *  - 정수를 10진 문자열로 보내려면 끝자리(v%10)부터 뽑아 버퍼에 담고 역순 송신.
 *  - 0 은 특수 처리(루프가 한 번도 안 돌기 때문).
 *  - 상태/카운터/ADC 값을 사람이 읽게 보낼 때 쓰는 핵심 헬퍼.
 *
 * 흔한 실수:
 *  - 역순 송신을 안 해서 자릿수가 뒤집히는 경우.
 *  - v==0 처리를 빠뜨려 아무것도 안 나오는 경우.
 *
 * 예상 동작(시뮬레이터): UART #1 에 "0 42 1000" 출력 후 줄바꿈.
 */

#define BAUD_BRR   7500u   /* 72MHz / 9600 */

static void usart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->CRH |=  (0xBu << ((9u - 8u) * 4u));    /* PA9 TX AF 푸시풀 */
    GPIOA->CRH |=  (0x4u << ((10u - 8u) * 4u));   /* PA10 RX 플로팅 입력 */

    USART1->BRR = BAUD_BRR;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

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

static void send_dec(uint16_t v)
{
    char buf[6];
    int i = 0;

    if (v == 0u) {
        send_char('0');
        return;
    }
    while (v > 0u) {
        buf[i++] = (char)('0' + (v % 10u));   /* 끝자리부터 */
        v /= 10u;
    }
    while (i > 0) {
        send_char(buf[--i]);                  /* 역순으로 송신 */
    }
}

int main(void)
{
    usart1_init();

    send_dec(0u);    send_char(' ');
    send_dec(42u);   send_char(' ');
    send_dec(1000u); send_str("\r\n");

    while (1) {
    }
}
