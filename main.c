#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Настройка радиоприемника nRF52840 для приема данных
void radio_init() {
  // Включение радиомодуля
  NRF_RADIO->POWER = RADIO_POWER_POWER_Enabled;

  // Настройка базовых параметров радио
  NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit; // Используем BLE с пропускной
                                               // способностью 1 Mbit

  // Настройка адреса приемника
  NRF_RADIO->PREFIX0 = 0x8E;     // Префикс адреса, например 0x8E
  NRF_RADIO->BASE0 = 0x89ABCDEF; // Базовый адрес, например 0x89ABCDEF
  NRF_RADIO->TXADDRESS = 0;      // Используем адрес 0 для передачи
  NRF_RADIO->RXADDRESSES =
      RADIO_RXADDRESSES_ADDR0_Enabled; // Включаем адрес 0 для приема

  // Настройка каналов и частоты
  NRF_RADIO->FREQUENCY = 2; // Настраиваем частоту на 2402 MHz (канал 2)
  NRF_RADIO->DATAWHITEIV =
      37; // Инициализация переменной дециметрии данных на канал 37

  // Настройка пакетного конфигурации
  NRF_RADIO->PCNF0 =
      (1 << RADIO_PCNF0_S0LEN_Pos) |
      (8 << RADIO_PCNF0_LFLEN_Pos); // Длина поля S0 и поля длины данных
  NRF_RADIO->PCNF1 =
      (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
      (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
      (24 << RADIO_PCNF1_MAXLEN_Pos); // Макс. длина пакета данных 24 байта

  // Настройка адреса начала приема
  NRF_RADIO->TASKS_RXEN = 1; // Включаем режим приема
}
void check_radio_state() {
  uint32_t state = NRF_RADIO->STATE;
  __asm__("NOP");

  // switch (state) {
  // case RADIO_STATE_STATE_RxIdle:
  //   printf("Радиомодуль в режиме ожидания приема (RXIDLE).\n");
  //   break;
  // case RADIO_STATE_STATE_Rx:
  //   printf("Радиомодуль активно принимает пакет (RXACT).\n");
  //   break;
  // case RADIO_STATE_STATE_TxIdle:
  //   printf("Радиомодуль в режиме ожидания передачи (TXIDLE).\n");
  //   break;
  // case RADIO_STATE_STATE_Tx:
  //   printf("Радиомодуль активно передает пакет (TXACT).\n");
  //   break;
  // default:
  //   printf("Неизвестное состояние радиомодуля.\n");
  //   break;
  // }
}
// Основная функция
int main(void) {
  // Инициализация радиомодуля
  radio_init();

  // Основной цикл программы
  while (1) {
    if (NRF_RADIO->EVENTS_DEVMISS) {
      NRF_RADIO->EVENTS_DEVMISS = 0; // Сбрасываем событие
      NRF_RADIO->TASKS_DISABLE =
          1; // Отключаем приемник, так как адрес не совпал
    } else {
      // Адрес совпал — продолжаем обработку пакета
    }
    // Ожидание приема данных
    if (NRF_RADIO->EVENTS_END) {
      // Обработка полученных данных
      uint8_t received_data[24];
      memcpy(received_data, (const uint8_t *)NRF_RADIO->PACKETPTR, 24);

      // Сброс событий радио
      NRF_RADIO->EVENTS_END = 0;
      NRF_RADIO->TASKS_START = 1; // Повторный запуск радио для приема данных
    }
    check_radio_state();
  }
}
