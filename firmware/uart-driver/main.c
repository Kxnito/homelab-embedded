#include <stdint.h>

#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_BASE      0x40020000
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

#define USART2_BASE     0x40004400
#define USART2_SR       (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0C))

void uart_init(void) {
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB1ENR |= (1 << 17);

    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);

    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |=  (7 << 8);

    USART2_BRR = 0x008B;

    USART2_CR1 |= (1 << 13) | (1 << 3);
}

void uart_send_char(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

int main(void)
{
    uart_init();

    while(1) {
        uart_send_string("Hello from STM32!\r\n");

        volatile uint32_t i;
        for(i = 0; i < 1000000; i++);
    }
}