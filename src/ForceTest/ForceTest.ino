#include <UMS3.h>

#define FORCE_VN 12
#define FORCE_VP 11

int force_vn = 0;
int force_vp = 0;
int vp_smooth_buf[8];
int vn_smooth_buf[8];
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
