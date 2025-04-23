#include "app_timer.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "clock.h"
#include "radio.h"

int main(void) {
  uint32_t err_code = NRF_SUCCESS;

  clock_initialization();

  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  APP_ERROR_CHECK(err_code);

  radio_init();

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
