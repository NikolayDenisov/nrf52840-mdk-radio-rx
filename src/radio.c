#include "radio.h"
#include "nrf_radio.h"
#include "radio_config.h"

static uint32_t packet;

void clock_initialization() {
  /* Start 16 MHz crystal oscillator */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  /* Wait for the external oscillator to start up */
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
    // Do nothing.
  }

  /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;

  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
    // Do nothing.
  }
}

void radio_init() {
  NRF_RADIO->TXPOWER = 0UL;
  NRF_RADIO->FREQUENCY = 7UL;
  NRF_RADIO->MODE = 0UL;

  NRF_RADIO->BASE0 = 0x00000000;
  NRF_RADIO->PREFIX0 = 0xAA;

  NRF_RADIO->TXADDRESS = 0x00UL;
  NRF_RADIO->RXADDRESSES = 0x01UL;

  NRF_RADIO->PCNF0 = 0x00000000;

  NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                     (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos) |
                     (PACKET_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos) |
                     (PACKET_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos) |
                     (PACKET_PAYLOAD_MAXSIZE << RADIO_PCNF1_MAXLEN_Pos);

  NRF_RADIO->PACKETPTR = (uint32_t)&packet;
}

uint32_t read_packet() {
  uint32_t result = 0;

  NRF_RADIO->EVENTS_READY = 0;
  NRF_RADIO->TASKS_RXEN = 1;
  while (!NRF_RADIO->EVENTS_READY) {
  }

  NRF_RADIO->EVENTS_END = 0;
  NRF_RADIO->TASKS_START = 1;
  while (!NRF_RADIO->EVENTS_END) {
  }

  result = packet;

  NRF_RADIO->EVENTS_DISABLED = 0;
  NRF_RADIO->TASKS_DISABLE = 1;
  while (!NRF_RADIO->EVENTS_DISABLED) {
  }

  return result;
}
