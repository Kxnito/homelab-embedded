#include <stdint.h>

// RCC
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))

// GPIOB
#define GPIOB_BASE      0x40020400
#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_AFRH      (*(volatile uint32_t *)(GPIOB_BASE + 0x24))

// GPIOA (for UART)
#define GPIOA_BASE      0x40020000
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

// I2C1
#define I2C1_BASE       0x40005400
#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))

// USART2
#define USART2_BASE     0x40004400
#define USART2_SR       (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0C))

// MPU-6050
#define MPU6050_ADDR    0x68
#define PWR_MGMT_1      0x6B
#define ACCEL_XOUT_H    0x3B

void uart_init(void) {
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB1ENR |= (1 << 17);
    GPIOA_MODER &= ~(3 << 4);
    GPIOA_MODER |=  (2 << 4);
    GPIOA_AFRL  &= ~(0xF << 8);
    GPIOA_AFRL  |=  (7 << 8);
    USART2_BRR   = 0x008B;
    USART2_CR1  |= (1 << 13) | (1 << 3);
}

void uart_send_char(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_send_string(const char *str) {
    while (*str) uart_send_char(*str++);
}

void uart_send_int(int32_t val) {
    char buf[16];
    int i = 0;
    if (val < 0) { uart_send_char('-'); val = -val; }
    if (val == 0) { uart_send_char('0'); return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (i--) uart_send_char(buf[i]);
}

void i2c_init(void) {
    RCC_AHB1ENR |= (1 << 1);      // GPIOB clock
    RCC_APB1ENR |= (1 << 21);     // I2C1 clock

    // PB8 = SCL, PB9 = SDA — alternate function mode
    GPIOB_MODER &= ~((3 << 16) | (3 << 18));
    GPIOB_MODER |=  ((2 << 16) | (2 << 18));

    // Open drain
    GPIOB_OTYPER |= (1 << 8) | (1 << 9);

    // AF4 for I2C1
    GPIOB_AFRH &= ~((0xF << 0) | (0xF << 4));
    GPIOB_AFRH |=  ((4 << 0) | (4 << 4));

    // Reset and configure I2C1
    I2C1_CR1 |= (1 << 15);
    I2C1_CR1 &= ~(1 << 15);

    I2C1_CR2  = 16;           // 16MHz peripheral clock
    I2C1_CCR  = 80;           // 100kHz I2C
    I2C1_TRISE = 17;
    I2C1_CR1 |= (1 << 0);    // Enable I2C
}

void i2c_write(uint8_t addr, uint8_t reg, uint8_t data) {
    // Start
    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & (1 << 0)));

    // Address
    I2C1_DR = addr << 1;
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    // Register
    I2C1_DR = reg;
    while (!(I2C1_SR1 & (1 << 7)));

    // Data
    I2C1_DR = data;
    while (!(I2C1_SR1 & (1 << 2)));

    // Stop
    I2C1_CR1 |= (1 << 9);
}

uint8_t i2c_read_byte(uint8_t addr, uint8_t reg) {
    // Start
    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & (1 << 0)));

    // Address write
    I2C1_DR = addr << 1;
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    // Register
    I2C1_DR = reg;
    while (!(I2C1_SR1 & (1 << 7)));
    while (!(I2C1_SR1 & (1 << 2)));

    // Restart
    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & (1 << 0)));

    // Address read
    I2C1_CR1 &= ~(1 << 10);  // disable ACK
    I2C1_DR = (addr << 1) | 1;
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    // Stop
    I2C1_CR1 |= (1 << 9);
    while (!(I2C1_SR1 & (1 << 6)));
    return I2C1_DR;
}

void i2c_read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & (1 << 0)));

    I2C1_DR = addr << 1;
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    I2C1_DR = reg;
    while (!(I2C1_SR1 & (1 << 7)));
    while (!(I2C1_SR1 & (1 << 2)));

    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & (1 << 0)));

    I2C1_CR1 |= (1 << 10);  // ACK
    I2C1_DR = (addr << 1) | 1;
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    for (uint8_t i = 0; i < len; i++) {
        if (i == len - 1) I2C1_CR1 &= ~(1 << 10);  // NACK last byte
        while (!(I2C1_SR1 & (1 << 6)));
        buf[i] = I2C1_DR;
    }
    I2C1_CR1 |= (1 << 9);  // Stop
}

int main(void) {
    uart_init();
    i2c_init();

    // Wake up MPU-6050
    i2c_write(MPU6050_ADDR, PWR_MGMT_1, 0x00);

    uart_send_string("MPU-6050 initialized\r\n");

    while (1) {
        uint8_t buf[6];
        i2c_read_bytes(MPU6050_ADDR, ACCEL_XOUT_H, buf, 6);

        int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t az = (int16_t)((buf[4] << 8) | buf[5]);

        uart_send_string("AX:");
        uart_send_int(ax);
        uart_send_string(" AY:");
        uart_send_int(ay);
        uart_send_string(" AZ:");
        uart_send_int(az);
        uart_send_string("\r\n");

        volatile uint32_t i;
        for (i = 0; i < 1000000; i++);
    }
}
