#include "app_timer.h"
#include "boards.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "radio_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_radio.h"

static uint32_t packet; /**< Packet to transmit. */

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

uint32_t read_packet() {
  uint32_t result = 0;

  NRF_RADIO->EVENTS_READY = 0;
  // Enable radio and wait for ready
  NRF_RADIO->TASKS_RXEN = 1;

  while (!NRF_RADIO->EVENTS_READY) {
    // wait
  }
  NRF_RADIO->EVENTS_END = 0;
  // Start listening and wait for address received event
  NRF_RADIO->TASKS_START = 1;

  // Wait for end of packet or buttons state changed
  while (!NRF_RADIO->EVENTS_END) {
    // wait
  }
  result = packet;
  NRF_RADIO->EVENTS_DISABLED = 0;
  // Disable radio
  NRF_RADIO->TASKS_DISABLE = 1;
  while (!NRF_RADIO->EVENTS_DISABLED) {
    // wait
  }
  return result;
}

void radio_init() {
  // Radio config
  NRF_RADIO->TXPOWER =
      (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
  NRF_RADIO->FREQUENCY = 7UL; // Frequency bin 7, 2407MHz
  NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);

  // Radio address config
  NRF_RADIO->PREFIX0 = 0xC3C28303;
  NRF_RADIO->PREFIX1 = 0xE3630023;

  NRF_RADIO->BASE0 = 0x80C4A2E6UL;
  NRF_RADIO->BASE1 = 0x91D5B3F7UL;

  NRF_RADIO->TXADDRESS =
      0x00UL; // Set device address 0 to use when transmitting
  NRF_RADIO->RXADDRESSES = 0x01UL; // Enable device address 0 to use to select
                                   // which addresses to receive

  // Packet configuration
  NRF_RADIO->PCNF0 =
      (0UL << RADIO_PCNF0_S1LEN_Pos) | (0UL << RADIO_PCNF0_S0LEN_Pos) |
      (0UL
       << RADIO_PCNF0_LFLEN_Pos); // lint !e845 "The right argument to operator
                                  // '|' is certain to be 0"

  // Packet configuration
  NRF_RADIO->PCNF1 =
      (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
      (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos) |
      (PACKET_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos) |
      (PACKET_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos) |
      (PACKET_PAYLOAD_MAXSIZE
       << RADIO_PCNF1_MAXLEN_Pos); // lint !e845 "The right argument to operator
                                   // '|' is certain to be 0"
}

int main(void) {
  NRF_LOG_INFO("Denisov");
  uint32_t err_code = NRF_SUCCESS;

  clock_initialization();

  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  err_code = bsp_init(BSP_INIT_LEDS, NULL);
  APP_ERROR_CHECK(err_code);

  // Set radio configuration parameters
  radio_init();
  NRF_RADIO->PACKETPTR = (uint32_t)&packet;

  err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_OFF);
  NRF_LOG_INFO("Radio receiver example started.");
  NRF_LOG_INFO("Wait for first packet");
  APP_ERROR_CHECK(err_code);
  NRF_LOG_FLUSH();

  while (true) {
    uint32_t received = read_packet();

    NRF_LOG_INFO("Packet was received");
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("The contents of the package is %u", (unsigned int)received);
    NRF_LOG_FLUSH();
  }
}