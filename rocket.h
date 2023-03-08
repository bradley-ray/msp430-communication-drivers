#ifndef ROCKETLIB
#define ROCKETLIB

#include <stdint.h>

#define CLK_DIVISOR 12 // clk frequency divisor

typedef enum {
	RADIO,
	MEMORY,
} spi_device;

int spi_rocket_init(spi_device device);
void spi_rocket_transmit(spi_device device, uint8_t cmd, uint8_t n, uint8_t* params);
void spi_rocket_receive(spi_device device);

// TODO: add rest of cmds
enum radio_opcode {
	SetTx 							= 0x83, // Tx mode
	SetRx 							= 0x82, // Rx mode
	SetRxTxFallbackMode = 0x93, // RADIO mode after Tx or Rx
	WriteRegister				= 0x0d, // write register
	WriteRegister				= 0x1d, // read register
	WriteBuffer					= 0x0e, // write FIFO register
	ReadBuffer					= 0x1e, // read FIFO register
	SetDioIrqParams			= 0x08, // confiugre IRQs and DIO
	GetIrqStatus				= 0x12, // get values of triggered IRQ
	ClearIrqStatus			= 0x02, // clear IRQs
	GetStatus						= 0xc0, // return current status of RADIO
	GetRxBufferStatus		= 0x13, // return PayloadLengthRx(7:0),RxBufferPointer(7:0)
	GetPacketStatus			= 0x14, // return a lot of stuff
	GetDeviceErrors			= 0x17, // return errors that have occured on RADIO
	GetStats						= 0x10, // statistics of last receieved packets
	ResetStats					= 0x00, // reset value read by GetStats
};

void radio_init();
void radio_transmit(uint8_t n, uint8_t* data);
void radio_receive();

#endif
