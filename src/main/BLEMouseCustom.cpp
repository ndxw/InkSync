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

// // Mouse Report (Report ID 1)
  // 0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
  // 0x09, 0x02,        // USAGE (Mouse)
  // 0xa1, 0x01,        // COLLECTION (Application)
  // 0x09, 0x01,        // USAGE (Pointer)
  // 0xa1, 0x00,        // COLLECTION (Physical)
  // USAGE(1),            0x30, //     USAGE (X)
  // LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  // LOGICAL_MAXIMUM(2),  0xff,  //     LOGICAL_MAXIMUM (1920)
  // REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  // REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  // HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;

  // USAGE(1),            0x31, //     USAGE (Y)
  // LOGICAL_MAXIMUM(2),  0xff,  //     LOGICAL_MAXIMUM (1080)
  // LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  // REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  // REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  // HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;
  // END_COLLECTION(0),         //   END_COLLECTION
  // END_COLLECTION(0)          // END_COLLECTION

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),       0x0d, // USAGE_PAGE (Digitizers)
  USAGE(1),            0x02, // USAGE (Pen)
  COLLECTION(1),       0x01, // COLLECTION (Application)
  USAGE(1),            0x20, //   USAGE (Stylus)
  COLLECTION(1),       0x00, //   COLLECTION (Physical)
  //-------------------------------------------------- 
  USAGE(1),            0x42, //     USAGE (Tip Switch)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1),  0x01, //     LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),      0x01, //     REPORT_SIZE (1)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
  // ------------------------------------------------- Padding
  REPORT_SIZE(1),      0x07, //     REPORT_SIZE (7) 
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x03, //     INPUT (Constant, Variable, Absolute) ;7 bit padding
  // ------------------------------------------------- X/Y position, Wheel
  USAGE_PAGE(1),       0x01, //     USAGE_PAGE (Generic Desktop)
  USAGE(1),            0x30, //     USAGE (X)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00, //     LOGICAL_MAXIMUM (1920)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;

  USAGE(1),            0x31, //     USAGE (Y)
  LOGICAL_MAXIMUM(2),  0xff, 0x00,  //     LOGICAL_MAXIMUM (1080)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;
  // ------------------------------------------------- Pen pressure
  USAGE_PAGE(1),       0x0d, //     USAGE PAGE (Digitizers)
  USAGE(1),            0x30, //     USAGE (Barrel pressure)
  LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(2),  0xff, 0x00, //     LOGICAL_MAXIMUM (255)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
  HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
  // -------------------------------------------------
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

void BLEMouseCustom::end(void)
{
}

void BLEMouseCustom::click(uint8_t b)
{
  
}

void BLEMouseCustom::move(uint8_t x, uint8_t y, unsigned char pressure)

{
  if (this->isConnected())
  {
    uint8_t m[4];
    m[0] = (pressure>(unsigned char)120) ? 1 : 0;

    //hard-code to split pressure into bigger range
    if(pressure<140){
      pressure = 0;
    } else if (pressure<160){
      pressure = 50;
    } else if (pressure<180){
      pressure = 100;
    } else if (pressure<200){
      pressure = 150;
    }


    m[1] = x;
    m[2] = y;
    m[3] = pressure;
    this->inputMouse->setValue(m,4);
    this->inputMouse->notify();

  }
}

void BLEMouseCustom::buttons(uint8_t b)
{
  
}

void BLEMouseCustom::press(uint8_t b)
{
  buttons(_buttons | b);
}

void BLEMouseCustom::release(uint8_t b)
{
  buttons(_buttons & ~b);
}

bool BLEMouseCustom::isPressed(uint8_t b)
{
  if ((b & _buttons) > 0)
    return true;
  return false;
}

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
  //BLEMouseCustom* blePenInstance = (BLEMouseCustom *) pvParameter;
  //Set device name
  BLEDevice::init(bleMouseInstance->deviceName.c_str());
  BLEDevice::setMTU(512);
  BLEDevice::setPower(ESP_PWR_LVL_P9,ESP_BLE_PWR_TYPE_DEFAULT);
  //BLEDevice::init(blePenInstance->deviceName.c_str());
  //Create server for advertising device
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleMouseInstance->connectionStatus);

  //Create mouse device and set HID
  bleMouseInstance->hid = new BLEHIDDevice(pServer);
  bleMouseInstance->inputMouse = bleMouseInstance->hid->inputReport(0); // <-- input REPORTID from report map
  bleMouseInstance->connectionStatus->inputMouse = bleMouseInstance->inputMouse;

  //Create pen device and set HID
  //blePenInstance->hid = new BLEHIDDevice(pServer);
  //blePenInstance->inputMouse = blePenInstance->hid->inputReport(1); // <-- input REPORTID from report map
  //blePenInstance->connectionStatus->inputMouse = blePenInstance->inputMouse;
  
  //Set device manufacturer name
  bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer.c_str());
  //blePenInstance->hid->manufacturer()->setValue(blePenInstance->deviceManufacturer.c_str());

  //pnp(device_id, vendor_id, product_id, device_version)
  //hidinfo(country, device_id)
  bleMouseInstance->hid->pnp(0x05, 0xe502, 0xa111, 0x0210);
  bleMouseInstance->hid->hidInfo(0x00,0x05);
  
  //blePenInstance->hid->pnp(0x05, 0xe503, 0xa112, 0x0210);
  //blePenInstance->hid->hidInfo(0x00,0x05);

  //setup bluetooth security for reconnection
  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  //Set hid_report_descriptor for both devices
  bleMouseInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleMouseInstance->hid->startServices();

  //blePenInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  //blePenInstance->hid->startServices();

  bleMouseInstance->onStarted(pServer);
  //blePenInstance->onStarted(pServer);

  //start advertising device
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_DIGITAL_PEN);
  //need to get UUID so PC knows what type of device it is
  //pAdvertising->addServiceUUID(blePenInstance->hid->hidService()->getUUID());
  pAdvertising->addServiceUUID(bleMouseInstance->hid->hidService()->getUUID());
  pAdvertising->start();
  bleMouseInstance->hid->setBatteryLevel(bleMouseInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}