#include <UMS3.h>

// Definitions ///////////////////////////////////////////

#define SPI_MISO 37
#define SPI_MOSI 35
#define SPI_SCLK 36
#define SPI_NCS  34

//////////////////////////////////////////////////////////

static const int spiClk = 1000000;  // 1 MHz
static const int spiClkPeriodMicroseconds = 500000;


void setup() {

  Serial.begin(115200);
  Serial.println("setup");

  initSPI();
}

void loop() {

  digitalWrite(SPI_MISO, HIGH);
  delay(10);
  digitalWrite(SPI_MISO, LOW);
  delay(10);

  // uint8_t ret = SPIRead(compileSPIProtocol(0x00, 0b01011101, 0b00110011));
  // char buffer[64];
  // sprintf(buffer, "0x%X", ret);
  // Serial.println(buffer);

  // while(1){
  //   delay(3000);
  // }  
}

void initSPI() {
  pinMode(SPI_MISO, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_SCLK, OUTPUT);
  pinMode(SPI_NCS, OUTPUT);
  digitalWrite(SPI_SCLK, HIGH);
  digitalWrite(SPI_NCS, HIGH);
}

void SPIWrite(uint16_t data)
{
  pinMode(SPI_MISO, OUTPUT);
  digitalWrite(SPI_NCS, LOW);
  for (int i = 15; i <= 0; i--)
  {
    digitalWrite(SPI_SCLK, LOW);
    digitalWrite(SPI_MISO, bitRead(data, i));
    delayMicroseconds(spiClkPeriodMicroseconds);
    digitalWrite(SPI_SCLK, HIGH);
    delayMicroseconds(spiClkPeriodMicroseconds);
  }
  digitalWrite(SPI_NCS, HIGH);
}

uint8_t SPIRead(uint16_t data)
{
  uint8_t addrByte = (uint8_t)(data >> 8);
  uint8_t dataByte = (uint8_t)data;
  char buffer[32];
  sprintf(buffer, "addrByte = 0x%X", addrByte);
  Serial.println(buffer);
  sprintf(buffer, "dataByte = 0x%X", dataByte);
  uint8_t out = 0;
  pinMode(SPI_MISO, OUTPUT);
  //digitalWrite(SPI_NCS, LOW);

  for (int i = 15; i >= 8; i--) {
    digitalWrite(SPI_SCLK, LOW);
    digitalWrite(SPI_MISO, bitRead(data, i));
    delayMicroseconds(spiClkPeriodMicroseconds);
    digitalWrite(SPI_SCLK, HIGH);
    delayMicroseconds(spiClkPeriodMicroseconds);
  }

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

uint16_t compileSPIProtocol(uint8_t rw, uint8_t addr, uint8_t data) {
  uint16_t protocol = 0x0;
  protocol = (rw << 7) | addr;
  protocol = protocol << 8;
  protocol = protocol | data;
  return protocol;
}