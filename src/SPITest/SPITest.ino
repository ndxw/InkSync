#include <UMS3.h>

// Definitions ///////////////////////////////////////////

#define SPI_MISO 37
#define SPI_MOSI 35
#define SPI_SCLK 36
#define SPI_NCS  34

//////////////////////////////////////////////////////////

static const int spiClk = 1000000;  // 1 MHz
static const int spiClkPeriodMicroseconds = 10;


void setup() {

  Serial.begin(115200);
  Serial.println("setup");

  initSPI();
}

void loop() {

  uint8_t ret = SPIRead(0x01);
  char buffer[64];
  sprintf(buffer, "0x%X", ret);
  Serial.println(buffer);

  while(1){
    delay(3000);
  }  
}

void initSPI() {
  pinMode(SPI_MISO, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_SCLK, OUTPUT);
  pinMode(SPI_NCS, OUTPUT);
  digitalWrite(SPI_SCLK, HIGH);
  digitalWrite(SPI_NCS, HIGH);
}

void SPIWrite(uint8_t addr, uint8_t data)
{
  // place addr in most significant 8 bits
  uint16_t protocol = ((uint16_t)(addr | 0x80)) << 8;
  // place data in least significant 8 bits
  protocol |= data;

  pinMode(SPI_MISO, OUTPUT);
  digitalWrite(SPI_NCS, LOW);
  for (int i = 15; i <= 0; i--)
  {
    digitalWrite(SPI_SCLK, LOW);
    digitalWrite(SPI_MISO, bitRead(protocol, i));
    delayMicroseconds(spiClkPeriodMicroseconds);
    digitalWrite(SPI_SCLK, HIGH);
    delayMicroseconds(spiClkPeriodMicroseconds);
  }
  digitalWrite(SPI_NCS, HIGH);
}

uint8_t SPIRead(uint8_t addr)
{
  uint8_t addrByte = addr & 0b01111111;
  // char buffer[32];
  // sprintf(buffer, "addrByte = 0x%X", addrByte);
  // Serial.println(buffer);
  uint8_t out = 0;
  pinMode(SPI_MISO, OUTPUT);
  digitalWrite(SPI_NCS, LOW);

  for (int i = 7; i >= 0; i--) {
    digitalWrite(SPI_SCLK, LOW);
    digitalWrite(SPI_MISO, bitRead(addrByte, i));
    delayMicroseconds(spiClkPeriodMicroseconds);
    digitalWrite(SPI_SCLK, HIGH);
    delayMicroseconds(spiClkPeriodMicroseconds);
  }

  // begin receiving byte
  pinMode(SPI_MISO, INPUT);
  
  for (int i = 7; i >= 0; i--) {
    digitalWrite(SPI_SCLK, LOW);
    delayMicroseconds(spiClkPeriodMicroseconds);
    digitalWrite(SPI_SCLK, HIGH);
    delayMicroseconds(spiClkPeriodMicroseconds);
    bitWrite(out, i, digitalRead(SPI_MISO));
  }
  
  pinMode(SPI_MISO, OUTPUT);
  digitalWrite(SPI_NCS, HIGH);
  return out;
}