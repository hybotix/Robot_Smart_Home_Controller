/*
  Raspberry Pi to Arduino Portenta C33 Pin Mapping

        Raspberry Pi          Portenta C33 / Raspberry Pi Aliases
*/
#define GPIO_02               29              //  Does NOT function as a GPIO
#define SDA1                  GPIO_02
#define SPI3_MOSI             GPIO_02

#define GPIO_03               40
#define SCL1                  GPIO_03
#define SPI3_SCLK             GPIO_03

#define GPIO_04                0              //  Does NOT function as a GPIO
#define SDA3                  GPIO_04
#define TX3                   GPIO_04

#define GPIO_05               93              //  Does NOT function as a GPIO
#define GPIO_06               33

#define GPIO_07               12              //  Does NOT function as a GPIO
#define SPI0_CS1              GPIO_07

#define GPIO_08                2              //  Does NOT function as a GPIO
#define SPI0_CS0              GPIO_08

#define GPIO_09               45              //  GPIO output works
#define SPI0_MISO             GPIO_09

#define GPIO_10               46
#define SPI0_MOSI             GPIO_10

#define GPIO_11               47              //  GPIO output works
#define SPI0_SCLK             GPIO_11

#define GPIO_12               92              //  Does NOT function as a GPIO
#define GPIO_13                4              //  Does NOT function as a GPIO

#define GPIO_16                5              //  GPIO output works
#define SPI1_CS0              GPIO_16

#define GPIO_17                2
#define SPI1_CS1              GPIO_17

#define GPIO_18               63              //  GPIO output works

#define GPIO_19               64              //  GPIO output works
#define SPI1_MISO             GPIO_19

#define GPIO_20               65              //  GPIO output works
#define SPI1_MOSI             GPIO_20

#define GPIO_21               66              //  GPIO output works
#define SPI1_SCLK             GPIO_21

#define GPIO_22               80              //  Does NOT function as a GPIO
#define SDA6                  GPIO_22

#define GPIO_23               78              //  Does NOT function as a GPIO
#define GPIO_24               79              //  Does NOT function as a GPIO
#define GPIO_25                1
#define GPIO_26                6              //  GPIO output works
#define GPIO_27               33              //  GPIO output works

#define SDA0                  11              //  I2C EEPROM
#define SCL0                  12              //  I2C EEPROM
#define TX0                   53              //  Does NOT function as a GPIO
#define RX0                   54              //  Does NOT function as a GPIO
