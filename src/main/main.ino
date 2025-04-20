#include <dummy.h>
#include <UMS3.h>
#include "spi.h"
#include "define.h"
#include <BleConnectionStatus.h>
#include <BLEMouseCustom.h>

#define FORCE_VP_PIN 11
#define FORCE_VN_PIN 12
#define NUM_READINGS 8

UMS3 ums3;

BLEMouseCustom bleMouseCustom("InkSync", "The InkSync Boys", 100);
char buffer[64];

// Optical sensor -----------------------------------------
SPI spi;
// sensor data
int8_t delta_xl; 
int8_t delta_yl; 
int8_t delta_xh; 
int8_t delta_yh; 
// full data
int16_t dx = 0; 
int16_t dy = 0; 

// Force sensor -------------------------------------------
int8_t vp_readings[NUM_READINGS];
int8_t vn_readings[NUM_READINGS];
int16_t vp_total = 0;
int16_t vn_total = 0;
double vout=0;
int read_index = 0;
char print_buffer[64];
uint8_t force_out;
//Position
int16_t abs_x = 0;
int16_t abs_y = 0;
int scale = 1;
//Counter for smoothing readings
int count = 0;

void setup() {
  Serial.begin(115200);
  //Force Sensor
  for (int i = 0; i < NUM_READINGS; i++)
  {
    vp_readings[i] = 0;
    vn_readings[i] = 0;
  }

  // LED
  ums3.begin();
  ums3.setPixelBrightness(255/2);
  ums3.setPixelPower(true);

  // Optical sensor
  spi.init();
  int status = initOpticalSensor();
  if (status == -1)
  { 
    Serial.println("Incorrect product ID during initialization!"); 
    ESP.restart(); 
  }
  else if (status != 0)
  { 
    sprintf(buffer, "Initialization failed at step %d!", status);
    Serial.println(buffer);
    ESP.restart();
  }
  Serial.println("Optical sensor initialization successful");

  //bluetooth connection
  bleMouseCustom.begin();
  while(!bleMouseCustom.isConnected()){
    ums3.setPixelColor(0, 0, 255);
    delay(500);
    ums3.setPixelColor(0, 0, 0);
    delay(500);
  }
  //moved to 0,0
  bleMouseCustom.move(127,127,0);
  bleMouseCustom.pen(127,127,0);
}

void loop() {
  readOpticalSensor();
  force_out = readForceSensor();
  
  //discard 8 samples of movement to slow movement of pen artificially
    if(count>8){
      //increase count for absolute position
      abs_x += dx;
      abs_y += dy;
      count=0;
    } else {
      count++;
    }

    //virtual boundaries (uint8_t sent over BLE, so 255 max)
    if(abs_x > 255*scale){
      abs_x = 255*scale;
    }
    if(abs_x < 0){
      abs_x = 0;
    }
    if(abs_y > 255*scale){
      abs_y = 255*scale;
    }
    if(abs_y < 0){
      abs_y = 0;
    }

    //send mouse data (mouse scaling is set to 2x position of pen by OS)
    bleMouseCustom.move(abs_x/(2*scale), abs_y/(2*scale), 0);
    //send pen data
    bleMouseCustom.pen(abs_x/scale, abs_y/scale, force_out);
}

/*
Initializes the PAW3040 optical sensor.
See 7.1.3 and Table 11 in datasheet.

Returns:
     0 - successful initialization
    -1 - incorrect product_ID1
     n - initialization failed at step n
*/
int initOpticalSensor()
{
  if (spi.read(PRODUCT_ID1) != 0x31){ 
    Serial.println("Product ID: ");
    Serial.println(spi.read(0x00)); 
    return -1; 
    }

  uint8_t initAddresses[] = {0x7F,0x05,0x09,0x51,0x0D,0x0E,0x07,0x1B,0x2E,0x32,0x33,0x34,0x36,0x38,0x39,0x3E,0x44,0x4A,0x4F,0x52,0x57,0x59,0x5B,0x5C,0x7F,0x00,0x07,0x20,0x21,0x23,0x2F,0x37,0x3B,0x43,0x59,0x5A,0x5C,0x5E,0x7F,0x51,0x7F,0x05,0x06,0x07,0x08,0x7F,0x05,0x53,0x7F,0x00,0x09,0x0A,0x0B,0x0C,0x0D,0x12,0x14,0x16,0x18,0x19,0x1A,0x1B,0x20,0x22,0x24,0x26,0x2F,0x30,0x3D,0x3E,0x7F,0x34,0x7F,0x00,0x02,0x03,0x06,0x0F,0x14,0x35,0x36,0x46,0x47,0x4B,0x7F,0x09,0x4B,0x7F,0x09};
  uint8_t initValues[]    = {0x00,0xA8,0x5A,0x06,0x1F,0x1F,0x00,0x42,0x40,0x40,0x02,0x00,0xE0,0xA0,0x01,0x14,0x02,0xE0,0x02,0x0D,0x03,0x03,0x03,0xFF,0x01,0x25,0x78,0x00,0x40,0x00,0x64,0x30,0x64,0x0A,0x01,0x01,0x04,0x04,0x02,0x02,0x03,0x0C,0x0C,0x0C,0x0C,0x04,0x01,0x08,0x05,0x02,0x01,0x1C,0x24,0x1C,0x24,0x08,0x02,0x02,0x1C,0x24,0x1C,0x24,0x08,0x02,0x02,0x88,0x7C,0x07,0x00,0x98,0x06,0x03,0x07,0x01,0xC4,0x13,0x0C,0x0A,0x02,0x39,0x3F,0x03,0x0F,0x97,0x00,0x00,0x97,0x00,0x00};

  for (int i = 0; i < 86; i++)
  {
    spi.write(initAddresses[i], initValues[i]);

    if (initAddresses[i] != BANK_SWITCH)
    {
      // read back value to verify successful write
      if (spi.read(initAddresses[i]) != initValues[i]){ return i+1; }
    }
  }
  return 0;
}


int readOpticalSensor(){
  // reading motion status before position register
  if (((spi.read(MOTION_STATUS)) >> 7) == 1)
  {
    delta_xl = spi.read(DELTA_XL);
    delta_yl = spi.read(DELTA_YL);
    delta_xh = spi.read(DELTA_XH); 
    delta_yh = spi.read(DELTA_YH); 

    dx = -(((int16_t)delta_xh << 8) | delta_xl);
    dy = -(((int16_t)delta_yh << 8) | delta_yl);

  }
  else
  { 
    dx = 0;
    dy = 0;
  }
  return 0;
}

int readForceSensor(){
  //takes 8 readings and averages for force output
  vp_total -= vp_readings[read_index];
  vn_total -= vn_readings[read_index];
  vp_readings[read_index] = analogRead(FORCE_VN_PIN);
  vn_readings[read_index] = analogRead(FORCE_VP_PIN);
  vp_total += vp_readings[read_index];
  vn_total += vn_readings[read_index];
  read_index++;
  if (read_index >= NUM_READINGS) read_index = 0;

  //unpressed is around -2
  //pressed goes to 15

  //normalize to range 0 to 255
  vout = ((((vp_total/8) - (vn_total/8))-(-15.0))/30.0)*255;

  return (uint8_t)vout;
}


