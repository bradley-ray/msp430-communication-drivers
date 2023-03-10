#include <msp430.h>
#include "driverlib.h"
#include "rocket.h"

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    bool result = spi_rocket_init(RADIO);
    if (result == STATUS_FAIL) {
        __delay_cycles(10);
    }

    while (1) {
        // TODO: test & make sure spi is enabled correctly
        spi_rocket_transmit(RADIO, SetRxTxFallbackMode, 0, NULL);
        __delay_cycles(1);
    }
}
