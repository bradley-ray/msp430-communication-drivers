#ifndef __ROCKETLIB__
#define __ROCKETLIB__

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
	SetSleep 				= 0x84, // set Sleep mode
	SetStandby	 			= 0x80, // set Standby mode
	SetFs 					= 0xc1, // set Freqency Synthesis mode
	SetTx 					= 0x83, // Tx mode
	SetRx 					= 0x82, // Rx mode
	StopTimerOnPreamble 	= 0x9f, // stop rx timeout on sync word/header preamble detect
	SetRxDutyCycle 			= 0x94, // stop rx timeout on sync word/header preamble detect
	SetTxContinousWave 		= 0xd1, // tx mode w/ inifnite carrier wave settings
	SetTxInfinitePreamble 	= 0xd2, // tx mode w/ inifnite preamable settings
	SetRegulatorMode 		= 0xd2, // set regulator mode (LDO, DC_DC+LDO, FS, RX, TX)
	SetPaConfig 			= 0x95, // set duty cycle, max power out
	SetRxTxFallbackMode 	= 0x93, // RADIO mode after Tx or Rx
	WriteRegister			= 0x0d, // write register
	ReadRegister			= 0x1d, // read register
	WriteBuffer				= 0x0e, // write FIFO register
	ReadBuffer				= 0x1e, // read FIFO register
	SetDioIrqParams			= 0x08, // confiugre IRQs and DIO
	GetIrqStatus			= 0x12, // get values of triggered IRQ
	ClearIrqStatus			= 0x02, // clear IRQs
	SetRfFrequency			= 0x86, // set rf freq
	SetPacketType			= 0x8a, // packet type according to modem/protocol
	GetPacketType			= 0x11, // packet type according to modem/protocol
	SetTxParams				= 0x8e, // set power and ramptime
	SetModulationParams		= 0x8b, // compute and set according to protocol
	SetPacketParams			= 0x8c, // set values on protocol modem
	SetBufferBaseAddress	= 0x8f, // set base address in data buffer for Tx & Rx
	SetLoRaSymbNumTimeout	= 0xa0, // number of symbols LoRa modem has to wait to validate lock
	GetStatus				= 0xc0, // return current status of RADIO
	GetRxBufferStatus		= 0x13, // return PayloadLengthRx(7:0),RxBufferPointer(7:0)
	GetPacketStatus			= 0x14, // return a lot of stuff
	GetDeviceErrors			= 0x17, // return errors that have occured on RADIO
	ClearDeviceErrors		= 0x07, // clear all errors
	GetStats				= 0x10, // statistics of last receieved packets
	ResetStats				= 0x00, // reset value read by GetStats
};

void radio_init();
uint8_t is_radio_busy();
void radio_transmit(uint8_t n, uint8_t* data);
void radio_receive();

#endif
