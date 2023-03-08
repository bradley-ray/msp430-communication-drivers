# MSP430 Communication Drivers
High level wrapper around MSP430 DriverLib for the communication interfaces on the msp430f5529.
Specifically for use in an electrical senior design project that includes the design of two boards: one multistage and dual deployment capable rocket controller and one telemetry reception board on the ground that interfaces via usb with a laptop/computer. 

Example uses found in `examples/` directory.

## Rocket Board
- 2 spi communication interfaces
  - 1 for communication with the transceiver module
  - 1 for communication with 32MB flash chip
- 1 i2c communication interface
  - for communication with imu and altimeter chips
- 1 uart communication interface
  - for communication with gps module

## Ground Board
- 1 spi communication interface
  - for communication with tranceiver module
- 1 usb communication interface
  - for communication with laptop/computer
