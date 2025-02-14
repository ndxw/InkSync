#include <UMS3.h>

#define FORCE_VN 2
#define FORCE_VP 1

int force_vn = 0;
int force_vp = 0;
char buffer[64];

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  force_vn = analogRead(FORCE_VN);
  force_vp = analogRead(FORCE_VP);

  sprintf(buffer, "Vn = %d, Vp = %d, Vout = %d\n", force_vn, force_vp, force_vp - force_vn);
  Serial.println(buffer);

  delay(200);
}
