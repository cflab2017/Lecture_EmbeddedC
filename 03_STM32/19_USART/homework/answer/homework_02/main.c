#include "stm32f10x.h"

/*
 * 정답 2 (MDK, STM32F103C8) — 소문자를 대문자로 바꿔 에코
 *
 * 핵심 포인트:
 *  - 수신 문자가 'a'~'z' 면 대문자로 변환 후 에코, 그 외는 그대로.
 *  - 변환 공식: c - 'a' + 'A' (소문자/대문자 ASCII 간격이 일정함을 이용).
 *
 * 흔한 실수:
 *  - 범위 판정 없이 모든 문자에 -32 를 적용해 숫자/기호까지 깨뜨리는 경우.
 *
 * 예상 동작(시뮬레이터): UART #1 에 READY 출력 후, 입력 소문자가 대문자로 에코.
 */

#define BAUD_BRR   7500u   /* 72MHz / 9600 */

static void usart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->CRH |=  (0xBu << ((9u - 8u) * 4u));    /* PA9  TX AF 푸시풀 */
    GPIOA->CRH |=  (0x4u << ((10u - 8u) * 4u));   /* PA10 RX 플로팅 입력 */

    USART1->BRR = BAUD_BRR;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void usart1_send_char(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = (uint16_t)c;
}

static void usart1_send_str(const char *s)
{
    while (*s) {
        usart1_send_char(*s++);
    }
}

int main(void)
{
    usart1_init();
    usart1_send_str("READY\r\n");

    while (1) {
        if (USART1->SR & USART_SR_RXNE) {
            char c = (char)USART1->DR;
            if (c >= 'a' && c <= 'z') {
                c = (char)(c - 'a' + 'A');   /* 소문자 → 대문자 */
            }
            usart1_send_char(c);
        }
    }
}
