#include <cstdint>
#include <Arduino.h>

#define OMGS3

#ifdef OMGS3
#define SPI_MISO 4
#define SPI_MOSI 3
#define SPI_SCLK 2
#define SPI_NCS 1
#else
#define SPI_MISO 37
#define SPI_MOSI 35
#define SPI_SCLK 36
#define SPI_NCS  34
#endif

#define CLK_PERIOD_MICRO 2

class SPI
{
  private:
    uint8_t pinMosi;
    uint8_t pinMiso;
    uint8_t pinSclk;
    uint8_t pinNcs;
  public:
    SPI();
    SPI(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t ncs);
    void init();
    void write(uint8_t addr, uint8_t byte);
    uint8_t read(uint8_t addr);
};


