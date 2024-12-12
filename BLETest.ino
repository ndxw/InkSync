#include <UMS3.h>

#include <BleConnectionStatus.h>
#include <BleMouse.h>

#include <math.h>

BleMouse bleMouse("Test", "Test manufacturer", 100);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  bleMouse.begin();

  while(!bleMouse.isConnected()){
    delay(1000);
  }

  delay(5000);
}

void loop() {

  bleMouse.press(MOUSE_LEFT);
  bleMouse.move(100, 0);
  bleMouse.release(MOUSE_LEFT);

  bleMouse.move(0, 100);

  bleMouse.press(MOUSE_LEFT);
  bleMouse.move(-100, 0);
  bleMouse.release(MOUSE_LEFT);

  bleMouse.move(0, -100);

  delay(2000);


}
