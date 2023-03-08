#include "../rocket.h"

void main(void)
{
	bool result = spi_rocket_init(RADIO);
	if (result == STATUS_FAIL) {
		__delay_cycles(10);
	}

	while (1) {
		// TODO: test & make sure spi is enabled correctly
		spi_rocket_transmit(RADIO, SetStandBy, 0, NULL);
		__delay_cylces(10);
	}
}
