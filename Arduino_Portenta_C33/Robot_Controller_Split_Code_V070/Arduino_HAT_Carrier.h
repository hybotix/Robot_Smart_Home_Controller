/*
*  Raspberry Pi to Arduino Portenta C33 Pin Mapping
*
*  Copyright (c) by Dale Weber <hybotics@hybotics.dev> 2024
*
*        Raspberry Pi Portenta C33 / Raspberry Pi Aliases
*/

#ifndef ARDUINO_HAT_CARRIER_H
#define ARDUINO_HAT_CARRIER_H

/*

New information from Arduino.cc

Portenta C33 <---> Raspberry Pi 4B
39 <---> GPIO2 (SDA)
40 <---> GPIO3 (SCL)
0 <---> GPIO4 (GPCLK0)
29 <---> GPIO17
33 <---> GPIO27
80 <---> GPIO22
46 <---> GPIO10 (MOSI)
45 <---> GPIO9 (MISO)
47 <---> GPIO11 (SCLK)
11 <---> GPIO0 (ID_SD)
13 <---> GPIO5
3 <---> GPIO6
4 <---> GPIO13 (PWM1)
64 <---> GPIO19 (PCM_FS)
6 <---> GPIO26
53 <---> GPIO14 (TXD)
54 <---> GPIO15 (RXD)
63 <---> GPIO18 (PCM_CLK)
78 <---> GPIO23
79 <---> GPIO24
1 <---> GPIO25
48 <---> GPIO8 (CE0)
2 <---> GPIO7 (CE1)
12 <---> GPIO1 (ID_SC)
14 <---> GPIO12 (PWM0)
5 <---> GPIO16
65 <---> GPIO20 (PCM_DIN)
66 <---> GPIO21 (PCM_DOUT)
*/

#define D7    11
#define D8    11
#define D9    11
#define D10   11
#define D11   11
#define D12   11
#define D13   11
#define D14   11

/*
*
*   Raspberry Pi              Portenta C33 / Raspberry Pi Aliases
*/

#define GPIO_00               11              //
#define ID_SD                 GPIO_00

#define GPIO_01               12              //
#define ID_SC                 GPIO_01

#define GPIO_02               39              //
#define SDA1                  GPIO_02
#define SPI3_MOSI             GPIO_02

#define GPIO_03               40              //
#define SCL1                  GPIO_03
#define SPI3_SCLK             GPIO_03

#define GPIO_04                0              //
#define SDA3                  GPIO_04
#define TX3                   GPIO_04
#define GPCLK0                GPIO_04

#define GPIO_05               13              //
#define GPIO_06                3              //

#define GPIO_07                2              //
#define SPI0_CS1              GPIO_07

#define GPIO_08               48              //
#define SPI0_CS0              GPIO_08

#define GPIO_09               45              //
#define SPI0_MISO             GPIO_09

#define GPIO_10               46              //
#define SPI0_MOSI             GPIO_10

#define GPIO_11               47              //
#define SPI0_SCLK             GPIO_11

#define GPIO_12               14              //
#define PWM0                  GPIO_12

#define GPIO_13                4              //
#define PWM1                  GPIO_13

#define GPIO_14               53              //
#define TX                    GPIO_14

#define GPIO_15               54              //
#define RX                    GPIO_15

#define GPIO_16                5              //
#define SPI1_CS0              GPIO_16

#define GPIO_17               29              //
#define SPI1_CS1              GPIO_17

#define GPIO_18               63              //
#define PCM_CLK               GPIO_18

#define GPIO_19               64              //
#define SPI1_MISO             GPIO_19
#define PCM_FS                GPIO_19

#define GPIO_20               65              //
#define PCM_DIN               GPIO_20
#define SPI1_MOSI             GPIO_20

#define GPIO_21               66              //
#define PCM_DOUT              GPIO_21
#define SPI1_SCLK             GPIO_21

#define GPIO_22               80              //
#define SDA6                  GPIO_22

#define GPIO_23               78              //
#define GPIO_24               79              //
#define GPIO_25                1
#define GPIO_26                6              //
#define GPIO_27               33              //

#define SDA0                  11              //
#define SCL0                  12              //
#define TX0                   53              //
#define RX0                   54              //

#endif
