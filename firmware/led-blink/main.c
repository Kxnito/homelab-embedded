#include <stdint.h>

#define RCC_BASE    0x40023800
#define GPIOA_BASE  0x40020000

#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

void delay(volatile uint32_t count) {
    while(count--);
}

int main(void)
{
    RCC_AHB1ENR |= (1 << 0);

    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |=  (1 << 10);

    while(1) {
        GPIOA_ODR |=  (1 << 5);
        delay(500000);
        GPIOA_ODR &= ~(1 << 5);
        delay(500000);
    }
}