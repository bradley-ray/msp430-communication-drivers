#include <stdint.h>
#include <msp430.h>
#include "rocket.h"
#include "driverlib.h"

// (slau132y) volatile should be used when variable modified
// by non standard ctrl flow (like interrupts)
/*
 * modified by RADIO_DIO interrupt to indicate radio done w/ Tx or Rx
 */
volatile uint8_t radio_ready = 0;

// TODO: need to make sure to set clock phase & polarity correctly
// TODO: ensure doesn't violate minimal timings
/*
 * Initialize SPI for communication
 */
int spi_rocket_init(spi_device device)
{
    int result;
    USCI_B_SPI_initMasterParam paramA;
    USCI_A_SPI_initMasterParam paramB;
    switch (device) {
        case RADIO:
            //paramA = {0};
            paramA.selectClockSource = USCI_B_SPI_CLOCKSOURCE_SMCLK;
            paramA.clockSourceFrequency = UCS_getSMCLK();
            paramA.desiredSpiClock = CLK_DIVISOR;
            paramA.msbFirst = USCI_B_SPI_MSB_FIRST;
            paramA.clockPhase = USCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            paramA.clockPolarity = USCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            result = USCI_B_SPI_initMaster(USCI_B0_BASE, &paramA);
            if (STATUS_FAIL == result) {
                return result;
            }
            P2SEL &= ~BIT4; // set RADIO chip select low
            P3SEL |= BIT0 + BIT1 + BIT2; // set P3.0,P3.1,P3.2 in spi mode
            USCI_B_SPI_enable(USCI_B0_BASE);
            // USCI_B_SPI_enableInterrupt(USCI_B0_BASE, UCTXIE); // enable tx interrupt
            // USCI_B_SPI_enableInterrupt(USCI_B0_BASE, UCRXIE); // enable rx interrupt
            break;
        case MEMORY:
            //paramB = {0};
            paramB.selectClockSource = USCI_A_SPI_CLOCKSOURCE_SMCLK;
            paramB.clockSourceFrequency = UCS_getSMCLK();
            paramB.desiredSpiClock = CLK_DIVISOR;
            paramB.msbFirst = USCI_A_SPI_MSB_FIRST;
            paramB.clockPhase = USCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
            paramB.clockPolarity = USCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
            result = USCI_A_SPI_initMaster(USCI_A0_BASE, &paramB);
            if (STATUS_FAIL == result) {
                return result;
            }
            P2SEL &= ~BIT3; // set MEMORY chip select low
            P2SEL |= BIT7; // set P2.7 to spi mode
            P3SEL |= BIT3 + BIT4; // set P3.3, P3.4 in SPI Mode
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
 *      spi_device device: slave/target device (RADIO | MEMORY)
 *      uint8_t    opcode: cmd to send to device
 *      uint8_t         n: number of bytes in params
 *      uint8_t*   params: list of bytes to send as parameters
 */
void spi_rocket_transmit(spi_device device, uint8_t opcode, uint8_t n, uint8_t* params)
{
    int i = 0;
    switch (device) {
        case RADIO:
            while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE, UCTXIFG));
            USCI_B_SPI_transmitData(USCI_B0_BASE, opcode);
            for (i = 0; i < n; ++i) {
                while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE, UCTXIFG));
                USCI_B_SPI_transmitData(USCI_B0_BASE, params[i]);
            }
            break;
        case MEMORY:
            while (!USCI_A_SPI_getInterruptStatus(USCI_A0_BASE, UCTXIFG));
            USCI_A_SPI_transmitData(USCI_A0_BASE, opcode);
            for (i = 0; i < n; ++i) {
                while (!USCI_A_SPI_getInterruptStatus(USCI_A0_BASE, UCTXIFG));
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
    uint8_t data = 0;
    switch (device) {
        case RADIO:
            while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE, UCRXIFG));
            data = USCI_B_SPI_receiveData(USCI_B0_BASE);
            break;
        case MEMORY:
            while (!USCI_A_SPI_getInterruptStatus(USCI_A0_BASE, UCRXIFG));
            data = USCI_A_SPI_receiveData(USCI_A0_BASE);
            break;
    }
    return data;
}

// TODO: handle interrupts
/*
 * USCI_A0 Interrupt Table
 */
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void) {
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
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch(__even_in_range(UCB0IV,4)) {
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
 * Port 1 Vector
 */

// TODO: a lot of the radio tx & rx code can probably be placed into init
// TODO: give all the params bytes a name?
//           (but don't do this until after Tx/Rx are working)
/*
 * Initalize Radio
 */
void radio_init()
{
    // TODO: does the pullup need to be set on the microcontroller?
    P8DIR &= ~BIT2; // make sure RADIO_BUSY pin is set to input
}

/*
 * Check if radio is busy
 */
uint8_t is_radio_busy()
{
    return P8IN &= BIT2;
}

// TODO: all writes cause RADIO_BUSY to go high,
//              so need to wait until it goes low agian
/*
 * Radio Tx Procedure
 *       1. SetStandBy() -- if not in STDBY_RC
 *       2. SetPacketType() -- define protocol (LoRa | FSK)
 *       3. SetRfFrequency()
 *       4. SetPaConfig() -- config power amplification
 *       5. SetTxParams() -- power output and ramptime
 *       6. SetBufferBaseAddress() -- setting payload location
 *       7. WriteBuffer()
 *       8. SetModulationParameters()
 *       9. SetPacketParams()
 *      10. SetDioIrqParams() -- set TxDone to DIO(1|2|3) & set Timeout
 *      11. WriteRegister() -- write Sync word to register
 *      12. SetTx() -- once packet sent, chip goes in auto STDBY_RC
 *      13. Wait for TxDone or Timeout --
 *      14. Clear TxDone flag
 */
void radio_transmit(uint8_t n, uint8_t* data)
{
    uint8_t params[16];
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetStandBy, 0, NULL);
    params[0] = 0x01; // LoRa Mode
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetPacketType, 1, params);
    params[0] = 0x00; // TODO: determine what this needs to be
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetRfFrequency, 0, NULL);
    // this setup should mean output power max about +14dBm
    params[0] = 0x02;
    params[1] = 0x02;
    params[2] = 0x00;
    params[3] = 0x01;
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetPaConfig, 4, params);
    params[0] = 0x0e; // power 14dBm
    params[1] = 0x03; // rampup time of 80us
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetTxParams, 2, params);
    params[0] = 0x00; // TODO: need to determine data size to be sent
    params[1] = 0x00;
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetBufferBaseAddress, 2, params);
    // TODO: handle n >= 16
    memcpy(params+1, data, n);
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, WriteBuffer, n+1, params);
    // TODO: set modulation parameters
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetModulationParameters, 0, NULL);
    // TODO: set packet parameters
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetPacketParams, 0, NULL);
    params[0] = 0x02; // enable Timeout
    params[1] = 0x01; // enable TxDone
    params[2] = 0x02; // Timeout -> DIO1
    params[3] = 0x01; // TxDone -> DIO1
    int i = 0;
    for (i = 4; i < 16; ++i) {
        params[i] = 0x00;
    }
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, SetDioIrqParams, 8, params);
    params[0] = 0x07; // upper byte of sync word address
    params[1] = 0x40; // lower byte of sync word address
    params[2] = 0x14; // set syncword/lora to private network
    params[3] = 0x24;
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, WriteRegister, 4, params);
    params[0] = 0x0f;
    params[1] = 0x42;
    params[2] = 0x40;
    while(!is_radio_busy());
    spi_rocket_transmit(RADIO, SetTx, 3, params);
    // TODO: finish implementing interrupt handling for RADIO_DIO
    while (!radio_ready);
    params[0] = 0x02; // clear Timeout IRQ
    params[1] = 0x01; // clear TxDone IRQ
    while (!is_radio_busy());
    spi_rocket_transmit(RADIO, ClearIrqStatus, 2, params);
    radio_ready = 0;
}

// TODO: implement Radio Rx
/*
 * Radio Rx Procedure
 *       1. SetStandBy() -- if not in STDBY_RC
 *       2. SetPacketType() -- define protocol (LoRa | FSK)
 *       3. SetRfFrequency()
 *       4. SetBufferBaseAddress() -- setting payload location
 *       5. SetModulationParameters()
 *       6. SetPacketParams()
 *       7. SetDioIrqParams() -- set RxDone to DIO(1|2|3) & set Timeout
 *       8. WriteRegister() -- write Sync word to register
 *       9. SetRx()
 *      10. Wait for RxDone or Timeout
 *      11. GetIrqStatus() -- need to check CRC, proceed if okay
 *      12. Clear RxDone flag
 *      13. GetRxBufferStatus() -- get length of payload and pointer
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
    // TODO: finish implementing interrupt handling for RADIO_DIO
    while (!radio_ready);
    spi_rocket_transmit(RADIO, GetIrqStatus, 0, NULL);
    spi_rocket_transmit(RADIO, ClearIrqStatus, 0, NULL);
    spi_rocket_transmit(RADIO, GetRxBufferStatus, 0, NULL);
    radio_ready = 0;
}

// TODO: Handle interrupt from RADIO_DIO
// #pragma INTERRUPT ( func_name )
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
    switch(__even_in_range(UCB0IV,4)) {
        case 0x00: // Vector 0: No interrupts
            break;
        case 0x10: // Vector 16
            radio_ready = 1;
            break;
        default: break;
    }
}
