#include "main.h"
#include <stdio.h>

uint8_t display_digits[] = {0x7E, 0x30, 0x6D, 0x79}; // 0 to 3
UART_HandleTypeDef huart2;

void display_digit(uint8_t num) {
    uint16_t segment_bits = display_digits[num];
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, (segment_bits >> 6) & 1); // A
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, (segment_bits >> 5) & 1); // B
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, (segment_bits >> 4) & 1); // C
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, (segment_bits >> 3) & 1); // D
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, (segment_bits >> 2) & 1); // E
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, (segment_bits >> 1) & 1); // F
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, (segment_bits >> 0) & 1); // G
}

uint8_t get_vacant_slots() {
    uint8_t vacant = 0;
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1)) vacant++;
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)) vacant++;
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)) vacant++;
    return vacant;
}

void send_vacancy_to_esp32(uint8_t count) {
    char msg[10];
    sprintf(msg, "Vacant:%d\n", count);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1) {
        uint8_t vacant = get_vacant_slots();
        display_digit(vacant);
        send_vacancy_to_esp32(vacant);
        HAL_Delay(1000);
    }
} 