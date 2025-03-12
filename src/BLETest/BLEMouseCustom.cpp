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

// static const uint8_t _hidReportDescriptor[] = {
//   USAGE_PAGE(1),       0x0d, // USAGE_PAGE (Digitizers)
//   USAGE(1),            0x02, // USAGE (Pen)
//   COLLECTION(1),       0x01, // COLLECTION (Application)
//   USAGE(1),            0x20, //   USAGE (Stylus)
//   COLLECTION(1),       0x00, //   COLLECTION (Physical)
//   //-------------------------------------------------- Mouse1
//   USAGE_PAGE(1),       0x09, //     USAGE_PAGE (Button)
//   USAGE_MINIMUM(1),    0x01, //     USAGE_MINIMUM (Button 1)
//   USAGE_MAXIMUM(1),    0x01, //     USAGE_MAXIMUM (Button 1)
//   LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
//   LOGICAL_MAXIMUM(1),  0x01, //     LOGICAL_MAXIMUM (1)
//   REPORT_SIZE(1),      0x01, //     REPORT_SIZE (1)
//   REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
//   HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
//   // ------------------------------------------------- Padding
//   REPORT_SIZE(1),      0x07, //     REPORT_SIZE (3) 
//   REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
//   HIDINPUT(1),         0x03, //     INPUT (Constant, Variable, Absolute) ;7 bit padding
//   // ------------------------------------------------- X/Y position, Wheel
//   USAGE_PAGE(1),       0x01, //     USAGE_PAGE (Generic Desktop)
//   USAGE(1),            0x30, //     USAGE (X)
//   USAGE(1),            0x31, //     USAGE (Y)
//   LOGICAL_MINIMUM(1),  0x81, //     LOGICAL_MINIMUM (-127)
//   LOGICAL_MAXIMUM(1),  0x7f, //     LOGICAL_MAXIMUM (127)
//   REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
//   REPORT_COUNT(1),     0x02, //     REPORT_COUNT (2)
//   HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;3 bytes (X,Y,Wheel)
//   // ------------------------------------------------- Pen pressure
//   USAGE_PAGE(1),       0x0d, //     USAGE PAGE (Digitizers)
//   USAGE(1),            0x31, //     USAGE (Barrel pressure)
//   LOGICAL_MINIMUM(1),  0x00, //     LOGICAL_MINIMUM (0)
//   LOGICAL_MAXIMUM(2),  0xff, 0x00, //     LOGICAL_MAXIMUM (255)
//   REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
//   REPORT_COUNT(1),     0x01, //     REPORT_COUNT (1)
//   HIDINPUT(1),         0x02, //     INPUT (Data, Variable, Absolute)
//   // -------------------------------------------------
//   END_COLLECTION(0),         //   END_COLLECTION
//   END_COLLECTION(0)          // END_COLLECTION
// };

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),       0x0d, // USAGE_PAGE (Digitizers)
  USAGE(1),            0x02, // USAGE (Pen)
  COLLECTION(1),       0x01, // COLLECTION (Application)
  USAGE(1),            0x20, //   USAGE (Stylus)
  COLLECTION(1),       0x00, //   COLLECTION (Physical)
  //-------------------------------------------------- 
  USAGE(1),            0x42, //     USAGE_PAGE (Tip Switch)
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
  USAGE(1),            0x31, //     USAGE (Y)
  LOGICAL_MINIMUM(1),  0x81, //     LOGICAL_MINIMUM (-127)
  LOGICAL_MAXIMUM(1),  0x7f, //     LOGICAL_MAXIMUM (127)
  REPORT_SIZE(1),      0x08, //     REPORT_SIZE (8)
  REPORT_COUNT(1),     0x02, //     REPORT_COUNT (2)
  HIDINPUT(1),         0x06, //     INPUT (Data, Variable, Relative) ;3 bytes (X,Y,Wheel)
  // ------------------------------------------------- Pen pressure
  USAGE_PAGE(1),       0x0d, //     USAGE PAGE (Digitizers)
  USAGE(1),            0x31, //     USAGE (Barrel pressure)
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
  _buttons = b;
  move(0,0,0);
  _buttons = 0;
  move(0,0,0);
}

void BLEMouseCustom::move(signed char x, signed char y, unsigned char pressure)
{
  if (this->isConnected())
  {
    uint8_t m[4];
    m[0] = _buttons;
    m[1] = x;
    m[2] = y;
    m[3] = pressure;
    this->inputMouse->setValue(m, 4);
    this->inputMouse->notify();
  }
}

void BLEMouseCustom::buttons(uint8_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
    move(0,0,0);
  }
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
  BLEDevice::init(bleMouseInstance->deviceName.c_str());
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleMouseInstance->connectionStatus);

  bleMouseInstance->hid = new BLEHIDDevice(pServer);
  bleMouseInstance->inputMouse = bleMouseInstance->hid->inputReport(0); // <-- input REPORTID from report map
  bleMouseInstance->connectionStatus->inputMouse = bleMouseInstance->inputMouse;

  bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer.c_str());

  bleMouseInstance->hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  bleMouseInstance->hid->hidInfo(0x00,0x02);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  bleMouseInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleMouseInstance->hid->startServices();

  bleMouseInstance->onStarted(pServer);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_MOUSE);
  pAdvertising->addServiceUUID(bleMouseInstance->hid->hidService()->getUUID());
  pAdvertising->start();
  bleMouseInstance->hid->setBatteryLevel(bleMouseInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}