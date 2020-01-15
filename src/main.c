#include "nrf_log_default_backends.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

int main(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("Hello world");

    while(true) {
        // do nothing
    }
}
