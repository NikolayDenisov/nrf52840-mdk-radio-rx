#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define RADIO_RXADDRESS 0xE7E7E7E7 // Простой адрес для приёма
#define PAYLOAD_LENGTH 1           // Длина полезной нагрузки

#define UART_TX_PIN 20

void uart_init(void) {
  NRF_P0->PIN_CNF[UART_TX_PIN] =
      (1 << 0) | // Output direction
      (1 << 1) | // Input buffer disconnected
      (0 << 2);  // Connect pull-up or pull-down resistors

  NRF_UART0->PSEL.TXD = UART_TX_PIN;

  NRF_UART0->PSEL.RTS = 0xFFFFFFFF;
  NRF_UART0->PSEL.CTS = 0xFFFFFFFF;

  NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
  NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
  NRF_UART0->TASKS_STARTTX = 1;
}

void uart_send_char(char c) {
  NRF_UART0->TXD = c;
  while (NRF_UART0->EVENTS_TXDRDY == 0) {
  }
  NRF_UART0->EVENTS_TXDRDY = 0;
}

void radio_init(void) {
  // Настройка приёмного адреса
  NRF_RADIO->RXADDRESSES = 1; // Используем первый адрес из ADDRESS0
  NRF_RADIO->PREFIX0 = RADIO_RXADDRESS >> 24;
  NRF_RADIO->BASE0 = RADIO_RXADDRESS & 0xFFFFFF;

  // Настройка длины полезной нагрузки
  NRF_RADIO->PCNF1 = (PAYLOAD_LENGTH << RADIO_PCNF1_MAXLEN_Pos);

  // Установка частоты канала
  NRF_RADIO->FREQUENCY = 7; // Канал 7

  // Настройка модуляции и скорости передачи данных
  NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit;

  // Включение CRC
  NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two;
  NRF_RADIO->CRCINIT = 0xFFFF;  // Начальное значение CRC
  NRF_RADIO->CRCPOLY = 0x11021; // Полином для CRC-16

  // Включение модуля RADIO
  NRF_RADIO->TASKS_RXEN = 1;
}

uint8_t radio_receive(void) {
  // Запуск приёма
  NRF_RADIO->TASKS_START = 1;

  // Ожидание завершения приёма
  while (NRF_RADIO->EVENTS_END == 0) {
  }

  // Сброс флага завершения приёма
  NRF_RADIO->EVENTS_END = 0;

  // Получение данных из буфера
  uint8_t data = *(uint8_t *)NRF_RADIO->PACKETPTR;
  if (data != 0x00 && data != 0xFF) {
    uart_send_char(data);
  }

  return data;
}

void clock_init(void) {
  // Включение HFXO
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  // Ожидание стабилизации HFXO
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    ;

  // Сброс флага события
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
}

int main(void) {
  // Инициализация тактирования
  clock_init();
  // Инициализация радиоприёмника
  uart_init();

  // Ожидание приёма данных
  char start_ch = 'k';
  uart_send_char(start_ch);
  // Бесконечный цикл
  while (true) {
    radio_init();

    radio_receive();
  }
}