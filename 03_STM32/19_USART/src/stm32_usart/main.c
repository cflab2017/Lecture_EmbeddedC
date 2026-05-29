#include "stm32f10x.h"

/*
 * 19강 예제 — USART1 송신 + 에코.
 * 대상: STM32F103C8, 컴파일러: Keil MDK-ARM.
 *
 * USART1: TX=PA9, RX=PA10 (APB2 버스). 9600 8N1.
 * 시작 시 인사 문자열을 보내고, 이후 받은 문자를 그대로 돌려보낸다(에코).
 * 보율 분주값 BRR = fCK / baud, USART1 의 fCK 는 PCLK2(=72MHz).
 *   BRR = 72000000 / 9600 = 7500
 */

#define BAUD_BRR   7500u   /* 72MHz / 9600 */

static void usart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    /* PA9 = TX: AF 푸시풀(0xB), PA10 = RX: 플로팅 입력(0x4). 둘 다 CRH. */
    GPIOA->CRH &= ~((0xFu << ((9u - 8u) * 4u)) | (0xFu << ((10u - 8u) * 4u)));
    GPIOA->CRH |=  (0xBu << ((9u - 8u) * 4u));    /* PA9  AF 푸시풀 */
    GPIOA->CRH |=  (0x4u << ((10u - 8u) * 4u));   /* PA10 플로팅 입력 */

    USART1->BRR = BAUD_BRR;                        /* 9600 baud */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;  /* 송/수신 + 인에이블 */
}

static void usart1_send_char(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) { }      /* 송신 버퍼 빌 때까지 대기 */
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
    usart1_send_str("Hello, STM32 UART!\r\n");

    while (1) {
        if (USART1->SR & USART_SR_RXNE) {          /* 수신 데이터 있음? */
            char c = (char)USART1->DR;             /* 읽으면 RXNE 자동 클리어 */
            usart1_send_char(c);                   /* 그대로 에코 */
        }
    }
}
