#include <stdint.h>
#include <msp430.h>
#include "rocket.h"
#include "driverlib.h"

// TODO: need to make sure to set clock phase & polarity correctly
/*
 * Initialize SPI for communication
 */
int spi_rocket_init(spi_device device)
{
	int result;
	switch (device) {
		case RADIO:
			USCI_B_SPI_initMasterParam param = {0};
			param.selectClockSource = USCI_B_SPI_CLOCKSOURCE_SMCLK;
			param.clockSourceFrequency = UCS_getSMCLK();
			param.desiredSpiClock = CLK_DIVISOR;
			param.msbFirst = USCI_B_SPI_MSB_FIRST;
			param.clockPhase = USCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
			param.clockPolarity = USCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
			result = USCI_B_SPI_initMaster(USCI_B0_BASE, &param);
			if (STATUS_FAIL == result) {
				return result;
			}
			P2SEL &= ~BIT4 // set RADIO chip select low
			P3SEL |= BIT0 + BIT1 + BIT2 // set P3.0,P3.1,P3.2 in spi mode
			USCI_B_SPI_enable(USCI_B0_BASE);
			// USCI_B_SPI_enableInterrupt(USCI_B0_BASE, UCTXIE); // enable tx interrupt
			// USCI_B_SPI_enableInterrupt(USCI_B0_BASE, UCRXIE); // enable rx interrupt
			break;
		case MEMORY:
			USCI_A_SPI_initMasterParam param = {0};
			param.selectClockSource = USCI_A_SPI_CLOCKSOURCE_SMCLK;
			param.clockSourceFrequency = UCS_getSMCLK();
			param.desiredSpiClock = CLK_DIVISOR;
			param.msbFirst = USCI_A_SPI_MSB_FIRST;
			param.clockPhase = USCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
			param.clockPolarity = USCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
			result = USCI_A_SPI_initMaster(USCI_A0_BASE, &param);
			if (STATUS_FAIL == result) {
				return result;
			}
			P2SEL &= ~BIT3 // set MEMORY chip select low
			P2SEL |= BIT7 // set P2.7 to spi mode
			P3SEL |= BIT3 + BIT4 // set P3.3, P3.4 in SPI Mode
			USCI_A_SPI_enable(USCI_A0_BASE);
			// USCI_A_SPI_enableInterrupt(USCI_A0_BASE, UCTXIE); // enable tx interrupt
			// USCI_A_SPI_enableInterrupt(USCI_A0_BASE, UCRXIE); // enable rx interrupt
			break;
	};

	// __bis_SR_register(LPM0_bits + GIE);

	return result;
}

/*
 * Transmit data from microcontroller to slave/target
 *
 * 		spi_device device: slave/target device (RADIO | MEMORY)
 * 		uint8_t    opcode: cmd to send to device
 * 		uint8_t         n: number of bytes in params
 * 		uint8_t*   params: list of bytes to send as parameters
 */
void spi_rocket_transmit(spi_device device, uint8_t opcode, uint8_t n, uint8_t* params)
{
	switch (device) {
		case RADIO:
			while (!USCI_B_SPI_interruptStatus(USCI_B0_BASE, UCTXIFG));
			USCI_B_SPI_transmitData(USCI_B0_BASE, opcode);
			for (uint8_t i = 0; i < n; ++i) {
				while (!USCI_B_SPI_interruptStatus(USCI_B0_BASE, UCTXIFG));
				USCI_B_SPI_transmitData(USCI_B0_BASE, params[i]);
			}
			break;
		case MEMORY;
			while (!USCI_A_SPI_interruptStatus(USCI_A0_BASE, UCTXIFG));
			USCI_A_SPI_transmitData(USCI_A0_BASE, opcode);
			for (uint8_t i = 0; i < n; ++i) {
				while (!USCI_A_SPI_interruptStatus(USCI_A0_BASE, UCTXIFG));
				USCI_A_SPI_transmitData(USCI_A0_BASE, params[i]);
			}
			break;
	}
}

/*
 * Receive data from slave/target to microcontroller
 */
uint8_t spi_rocket_receive(spi_device device)
{
	switch (device) {
		case RADIO:
			while (!USCI_B_SPI_interruptStatus(USCI_B0_BASE, UCRXIFG));
			USCI_B_SPI_receiveData(USCI_B0_BASE, receiveData);
			break;
		case MEMORY;
			while (!USCI_A_SPI_interruptStatus(USCI_A0_BASE, UCRXIFG));
			USCI_A_SPI_receiveData(USCI_A0_BASE, receiveData);
			break;
	}
}

// TODO: handle interrupts
/*
 * USCI_A0 Interrupt Table
 */
#pragma vector = USCI_A0_VECTOR __interrupt void USCI_A0_ISR(void) {
	switch(__even_in_range(UCA0IV,4)) {
		case 0x00: // Vector 0: No interrupts
			break;
		case 0x02: // Vector 2: UCRXIFG
			break;
		case 0x04: // Vector 4: UCTXIFG
			break;
		default: break;
	}
}

/*
 * USCI_B0 Interrupt Table
 */
#pragma vector = USCI_B0_VECTOR __interrupt void USCI_B0_ISR(void) {
	switch(__even_in_range(UCB0IV,18)) {
		case 0x00: // Vector 0: No interrupts
			break;
		case 0x02: ... // Vector 2: UCRXIFG
			break;
		case 0x04: ... // Vector 4: UCTXIFG
			break;
		default: break;
	}
}

// TODO: a lot of the radio tx & rx code can probably be placed into init
// 			 (but don't do this until after Tx/Rx are working)
/*
 * Initalize Radio
 */
void radio_init()
{
}

/*
 * Radio Tx Procedure
 * 		 1. SetStandBy() -- if not in STDBY_RC
 * 		 2. SetPacketType() -- define protocol (LoRa | FSK)
 * 		 3. SetRfFrequency()
 * 		 4. SetPaConfig() -- config power amplification
 * 		 5. SetTxParams() -- power output and ramptime
 * 		 6. SetBufferBaseAddress() -- setting payload location
 * 		 7. WriteBuffer()
 * 		 8. SetModulationParameters()
 * 		 9. SetPacketParams()
 * 		10. SetDioIrqParams() -- set TxDone to DIO(1|2|3) & set Timeout
 * 		11. WriteRegister() -- write Sync word to register
 * 		12. SetTx() -- once packet sent, chip goes in auto STDBY_RC
 * 		13. Wait for TxDone or Timeout -- 
 * 		14. Clear TxDone flag
 */
void radio_transmit(uint8_t n, uint8_t* data)
{
	uint8_t params[16] = {0};
	spi_rocket_transmit(RADIO, SetStandBy, 0, NULL);
	params[0] = 0x01 // LoRa Mode
	spi_rocket_transmit(RADIO, SetPacketType, 1, params);
	params[0] = 0x00 // TODO: determine what this needs to be
	spi_rocket_transmit(RADIO, SetRfFrequency, 0, NULL);
	// this setup should mean output power max about +14dBm 
	params[0] = 0x02;
	params[1] = 0x02;
	params[2] = 0x00;
	params[3] = 0x01;
	spi_rocket_transmit(RADIO, SetPaConfig, 4, params);
	params[0] = 0x0e; // power 14dBm
	params[1] = 0x03; // rampup time of 80us
	spi_rocket_transmit(RADIO, SetTxParams, 2, params);
	params[0] = 0x00; // TODO: need to determine data size to be sent
	params[1] = 0x00;
	spi_rocket_transmit(RADIO, SetBufferBaseAddress, 2, params);
	// TODO: handle n >= 16
	memcpy(params+1, data, n);
	spi_rocket_transmit(RADIO, WriteBuffer, n+1, params);
	// TODO: set modulation parameters
	spi_rocket_transmit(RADIO, SetModulationParameters, 0, NULL);
	// TODO: set packet parameters
	spi_rocket_transmit(RADIO, SetPacketParams, 0, NULL);
	params[0] = 0x02; // enable Timeout
	params[1] = 0x01; // enable TxDone
	params[2] = 0x02; // Timeout -> DIO1
	params[3] = 0x01; // TxDone -> DIO1
	for (uint8_t i = 4; i < 16; ++i) {
		params[i] = 0x00;
	}
	spi_rocket_transmit(RADIO, SetDioIrqParams, 8, params);
	params[0] = 0x07; // upper byte of sync word address
	params[1] = 0x40; // lower byte of sync word address
	params[2] = 0x14; // set syncword/lora to private network
	params[3] = 0x24;
	spi_rocket_transmit(RADIO, WriteRegister, 4, params);
	params[0] = 0x0f;
	params[1] = 0x42;
	params[2] = 0x40;
	spi_rocket_transmit(RADIO, SetTx, 3, params);
	// TODO: implement wait until interrupt
	params[0] = 0x02; // clear Timeout IRQ
	params[1] = 0x01; // clear TxDone IRQ
	spi_rocket_transmit(RADIO, ClearIrqStatus, 2, params);
}

// TODO: implement Radio Rx
/*
 * Radio Rx Procedure
 * 		 1. SetStandBy() -- if not in STDBY_RC
 * 		 2. SetPacketType() -- define protocol (LoRa | FSK)
 * 		 3. SetRfFrequency()
 * 		 4. SetBufferBaseAddress() -- setting payload location
 * 		 5. SetModulationParameters()
 * 		 6. SetPacketParams()
 * 		 7. SetDioIrqParams() -- set RxDone to DIO(1|2|3) & set Timeout
 * 		 8. WriteRegister() -- write Sync word to register
 * 		 9. SetRx()
 * 		10. Wait for RxDone or Timeout
 * 		11. GetIrqStatus() -- need to check CRC, proceed if okay
 * 		12. Clear RxDone flag
 * 		13. GetRxBufferStatus() -- get length of payload and pointer
 */
void radio_receive()
{
	spi_rocket_transmit(RADIO, SetStandBy, 0, NULL);
	spi_rocket_transmit(RADIO, SetPacketType, 0, NULL);
	spi_rocket_transmit(RADIO, SetRfFrequency, 0, NULL);
	spi_rocket_transmit(RADIO, SetBufferBaseAddress, 0, NULL);
	spi_rocket_transmit(RADIO, SetModulationParameters, 0, NULL);
	spi_rocket_transmit(RADIO, SetPacketParams, 0, NULL);
	spi_rocket_transmit(RADIO, SetDioIrqParams, 0, NULL);
	spi_rocket_transmit(RADIO, WriteRegister, 0, NULL);
	spi_rocket_transmit(RADIO, SetRx, 0, NULL);
	// wait()
	spi_rocket_transmit(RADIO, GetIrqStatus, 0, NULL);
	spi_rocket_transmit(RADIO, ClearIrqStatus, 0, NULL);
	spi_rocket_transmit(RADIO, GetRxBufferStatus, 0, NULL);
}
