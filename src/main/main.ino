#include "spi.h"
#include "define.h"
#include <BleConnectionStatus.h>
#include <BleMouse.h>

SPI spi();
BleMouse bleMouse("InkSync", "The InkSync Boys", 100);

void setup() {
  Serial.begin(115200);
  spi.init();
}

void loop() {

  char buffer[64];
  // use WRITE_PROTECT register to test spi.write()
  uint8_t ret = spi.read(WRITE_PROTECT);
  sprintf(buffer, "WP = 0x%X", ret);
  Serial.println(buffer);

  while(1){
    delay(3000);
  }  
}

bool initOpticalSensor()
{
  uint8_t productID1 = spi.read(PRODUCT_ID1);
  if (productID1 != 0x31) return false;

  // do we need to switch register banks for init settings??

  // 1-10
  spi.write(0x7F, 0x00);
  spi.write(0x05, 0xA8);
  spi.write(0x09, 0x5A);
  spi.write(0x51, 0x06);
  spi.write(0x0D, 0x1F);
  spi.write(0x0E, 0x1F);
  spi.write(0x07, 0x00);
  spi.write(0x1B, 0x42);
  spi.write(0x2E, 0x40);
  spi.write(0x32, 0x40);
  // 11-20
  spi.write(0x33, 0x02);
  spi.write(0x34, 0x00);
  spi.write(0x36, 0xE0);
  spi.write(0x38, 0xA0);
  spi.write(0x39, 0x01);
  spi.write(0x3E, 0x14);
  spi.write(0x44, 0x02);
  spi.write(0x4A, 0xE0);
  spi.write(0x4F, 0x02);
  spi.write(0x52, 0x0D);
  // 21-30
  spi.write(0x57, 0x03);
  spi.write(0x59, 0x03);
  spi.write(0x5B, 0x03);
  spi.write(0x5C, 0xFF);
  spi.write(0x7F, 0x01);
  // 31-40

  // 41-50

  // 51-60

  // 61-70

  // 71-80

  // 80-86

  return true;
}
