#include <UMS3.h>

#define FORCE_VN 12
#define FORCE_VP 11
#define NUM_READINGS 8

int vp_readings[NUM_READINGS];
int vn_readings[NUM_READINGS];
int vp_total = 0;
int vn_total = 0;
int vp_avg = 0;
int vn_avg = 0;
int read_index = 0;
char print_buffer[64];


void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_READINGS; i++)
  {
    vp_readings[i] = 0;
    vn_readings[i] = 0;
  }
  delay(1000);
}

void loop() {
  vp_total -= vp_readings[read_index];
  vn_total -= vn_readings[read_index];
  vp_readings[read_index] = analogRead(FORCE_VN);
  vn_readings[read_index] = analogRead(FORCE_VP);
  vp_total += vp_readings[read_index];
  vn_total += vn_readings[read_index];
  read_index++;
  if (read_index >= readings_count) read_index = 0;

  vp_avg = vp_total >> 3;
  vn_avg = vn_total >> 3;

  sprintf(print_buffer, "Vn = %d, Vp = %d, Vout = %d\n", vn_avg, vp_avg, vp_avg - vn_avg);
  Serial.println(print_buffer);

  delay(200);
}


