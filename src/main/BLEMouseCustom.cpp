#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"

#include "BleConnectionStatus.h"
#include "BLEMouseCustom.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif

static const uint8_t _hidReportDescriptor[] = {
//Mouse
//-------------------------------------------------- Start Collections
  0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
  0x09, 0x02,        // USAGE (Mouse)
  0xa1, 0x01,        // COLLECTION (Application)
  0x09, 0x01,        // USAGE (Pointer)
  0xa1, 0x00,        // COLLECTION (Physical)
  0x85, 0x01,        // Report ID (1)
//-------------------------------------------------- Mouse Buttons
  0x05, 0x09,        // Usage Page (Button)
  0x19, 0x01,        // USAGE_MINIMUM (Button 1)
  0x29, 0x03,        // USAGE_MAXIMUM (Button 3)
  0x15, 0x00,        // Logical Minimum (0)
  0x25, 0x01,        // Logical Maximum (1)
  0x75, 0x01,        // Report Size (1)
  0x95, 0x03,        // Report Count (3)
  0x81, 0x02,        // Input (Data, Variable, Absolute)
//-------------------------------------------------- Bit Padding  
  0x05, 0x01,                //     USAGE_PAGE (Generic Desktop)
  REPORT_SIZE(1),      0x05, //     REPORT_SIZE (7) 
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x03, //     INPUT (Constant, Variable, Absolute) ;7 bit padding
//-------------------------------------------------- X
  USAGE(1),            0x30, //     USAGE (X)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00,//LOGICAL_MAXIMUM (255)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Relative) ;
//-------------------------------------------------- Y
  USAGE(1),            0x31, //     USAGE (Y)
  LOGICAL_MINIMUM(1),  0x00,//      LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00,//LOGICAL_MAXIMUM (1920)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Relative) ;
//--------------------------------------------------  End Collections
  END_COLLECTION(0),         //     END_COLLECTION
  END_COLLECTION(0),         //     END_COLLECTION
//-------------------------------------------------- 
//Pen
//-------------------------------------------------- Start Collections
  USAGE_PAGE(1),       0x0d, //     USAGE_PAGE (Digitizers)
  USAGE(1),            0x02, //     USAGE (Pen)
  COLLECTION(1),       0x01, //     COLLECTION (Application)
  USAGE(1),            0x20, //     USAGE (Stylus)
  COLLECTION(1),       0x00, //     COLLECTION (Physical)
  0x85, 0x02,                //     Report ID (2)
  //-------------------------------------------------- Tip Switch (Hover or pen down)
  USAGE(1),            0x42, //     USAGE (Tip Switch)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1),  0x01, //     LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),      0x01, //     REPORT_SIZE (1)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
  // ------------------------------------------------- Bit Padding
  REPORT_SIZE(1),      0x07, //     REPORT_SIZE (7) 
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x03, //     INPUT (Constant, Variable, Absolute)
  // ------------------------------------------------- X
  USAGE_PAGE(1),       0x01, //     USAGE_PAGE (Generic Desktop)
  USAGE(1),            0x30, //     USAGE (X)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00,//LOGICAL_MAXIMUM (1920)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative)
// ------------------------------------------------- Y
  USAGE(1),            0x31, //     USAGE (Y)
  LOGICAL_MAXIMUM(2),  0xff, 0x00,//LOGICAL_MAXIMUM (1080)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative)
  // ------------------------------------------------- Pen pressure
  USAGE_PAGE(1),       0x0d, //     USAGE PAGE (Digitizers)
  USAGE(1),            0x30, //     USAGE (Barrel pressure)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00, //     LOGICAL_MAXIMUM (255)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
  // ------------------------------------------------- End Collections
  END_COLLECTION(0),         //   END_COLLECTION
  END_COLLECTION(0)          // END_COLLECTION
};

BLEMouseCustom::BLEMouseCustom(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) : 
    _buttons(0),
    hid(0)
{
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;
  this->batteryLevel = batteryLevel;
  this->connectionStatus = new BleConnectionStatus();
}

void BLEMouseCustom::begin(void)
{
  xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
}

void BLEMouseCustom::end(void){}

void BLEMouseCustom::click(uint8_t b){}

void BLEMouseCustom::move(int8_t x_mouse, int8_t y_mouse, unsigned char click){
  if (this->isConnected())
  {
    uint8_t m[4];
    m[0] = click;
    m[1] = x_mouse;
    m[2] = y_mouse;

    this->inputMouse->setValue(m,4);
    this->inputMouse->notify();

  }
}

void BLEMouseCustom::pen(uint8_t x, uint8_t y, unsigned char pressure)

{
  if (this->isConnected())
  {
    uint8_t p[4];
    p[0] = (pressure>(unsigned char)140) ? 1 : 0;

    //Issue with pressure being different as pen tip wears out, so narmalizing varies
    //hard-code to split pressure into bigger range
    if(pressure<140){
      pressure = 0;
    } else if (pressure<150){
      pressure = 25;
    } else if (pressure<160){
      pressure = 50;
    } else if (pressure<170){
      pressure = 75;  
    } else if (pressure<180){
      pressure = 100;
    } else if (pressure<190){
      pressure = 125;
    } else if (pressure<200){
      pressure = 150;
    } else if (pressure<210){
      pressure = 175;
    } else if (pressure<220){
      pressure = 200;
    }

    p[1] = x;
    p[2] = y;
    p[3] = pressure;
    this->inputPen->setValue(p,4);
    this->inputPen->notify();
  }
}

//Unused, but here to match BLEMouse library used
void BLEMouseCustom::buttons(uint8_t b){}

void BLEMouseCustom::press(uint8_t b){}

void BLEMouseCustom::release(uint8_t b){}

bool BLEMouseCustom::isPressed(uint8_t b){}

bool BLEMouseCustom::isConnected(void) {
  return this->connectionStatus->connected;
}

void BLEMouseCustom::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
      this->hid->setBatteryLevel(this->batteryLevel);
}

void BLEMouseCustom::taskServer(void* pvParameter) {
  BLEMouseCustom* bleMouseInstance = (BLEMouseCustom *) pvParameter; //static_cast<BLEMouseCustom *>(pvParameter);
  
  //Set device name
  BLEDevice::init(bleMouseInstance->deviceName.c_str());
  
  //Max clock speed
  BLEDevice::setMTU(512);

  //Max power usage
  BLEDevice::setPower(ESP_PWR_LVL_P9,ESP_BLE_PWR_TYPE_DEFAULT);

  //Create server for advertising device
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleMouseInstance->connectionStatus);

  //Create mouse and pen device and set HIDs to tell PC which device corresponds to which input report
  bleMouseInstance->hid = new BLEHIDDevice(pServer);
  bleMouseInstance->inputMouse = bleMouseInstance->hid->inputReport(1); // <-- input REPORTID from report map
  bleMouseInstance->inputPen = bleMouseInstance->hid->inputReport(2); // <-- input REPORTID from report map
  bleMouseInstance->connectionStatus->inputMouse = bleMouseInstance->inputMouse;
  bleMouseInstance->connectionStatus->inputPen = bleMouseInstance->inputPen;
  
  //Set device manufacturer name
  bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer.c_str());

  //Set info for each device
  //Usage: pnp(device_id, vendor_id, product_id, device_version)
  //Usage: hidinfo(country, device_id)
  bleMouseInstance->hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  bleMouseInstance->hid->hidInfo(0x00,0x02);
  bleMouseInstance->hid->pnp(0x05, 0xe503, 0xa112, 0x0210);
  bleMouseInstance->hid->hidInfo(0x00,0x05);

  //setup bluetooth security for reconnection
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  //Set hid_report_descriptor for both devices
  bleMouseInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleMouseInstance->hid->startServices();

  bleMouseInstance->onStarted(pServer);

  //start advertising device
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_DIGITAL_PEN);

  //need to get UUID so PC knows what type of device it is
  pAdvertising->addServiceUUID(bleMouseInstance->hid->hidService()->getUUID());
  pAdvertising->start();
  bleMouseInstance->hid->setBatteryLevel(bleMouseInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}