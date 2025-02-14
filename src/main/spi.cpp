#include "spi.h"

SPI::SPI()
{
  this->pinMosi = SPI_MOSI;
  this->pinMiso = SPI_MISO;
  this->pinSclk = SPI_SCLK;
  this->pinNcs  = SPI_NCS;
}

SPI::SPI(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t ncs)
{
  this->pinMosi = mosi;
  this->pinMiso = miso;
  this->pinSclk = sclk;
  this->pinNcs  = ncs;
}

void SPI::init()
{
  pinMode(this->pinMiso, OUTPUT);
  pinMode(this->pinMosi, OUTPUT);
  pinMode(this->pinSclk, OUTPUT);
  pinMode(this->pinNcs, OUTPUT);
  digitalWrite(this->pinSclk, HIGH);
  digitalWrite(this->pinNcs, HIGH);
}

void SPI::write(uint8_t addr, uint8_t byte)
{
  // place addr in most significant 8 bits
  uint16_t protocol = ((uint16_t)(addr | 0x80)) << 8;
  // place data in least significant 8 bits
  protocol |= byte;

  pinMode(this->pinMiso, OUTPUT);
  digitalWrite(this->pinNcs, LOW);

  // sensor reads on rising edge as per SPI mode 3
  for (int i = 15; i >= 0; i--)
  {
    digitalWrite(this->pinSclk, LOW);
    digitalWrite(this->pinMiso, bitRead(protocol, i));
    delayMicroseconds(CLK_PERIOD_MICRO/2);
    digitalWrite(this->pinSclk, HIGH);
    delayMicroseconds(CLK_PERIOD_MICRO/2);
  }
  digitalWrite(this->pinNcs, HIGH);
}

uint8_t SPI::read(uint8_t addr)
{
  uint8_t addrByte = addr & 0x7F;
  uint8_t out = 0;
  
  pinMode(this->pinMiso, OUTPUT);
  digitalWrite(this->pinNcs, LOW);

  // sensor reads on rising edge as per SPI mode 3
  for (int i = 7; i >= 0; i--) {
    digitalWrite(this->pinSclk, LOW);
    digitalWrite(this->pinMiso, bitRead(addrByte, i));
    delayMicroseconds(CLK_PERIOD_MICRO/2);
    digitalWrite(this->pinSclk, HIGH);
    delayMicroseconds(CLK_PERIOD_MICRO/2);
  }

  // begin receiving byte
  pinMode(this->pinMiso, INPUT);
  
  for (int i = 7; i >= 0; i--) {
    digitalWrite(this->pinSclk, LOW);
    delayMicroseconds(CLK_PERIOD_MICRO/2);
    digitalWrite(this->pinSclk, HIGH);
    delayMicroseconds(CLK_PERIOD_MICRO/2);
    bitWrite(out, i, digitalRead(this->pinMiso));
  }
  
  pinMode(this->pinMiso, OUTPUT);
  digitalWrite(this->pinNcs, HIGH);
  return out;
}




