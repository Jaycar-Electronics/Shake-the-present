
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <U8g2lib.h>
#include <SPI.h>

MMA8452Q accel;
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI oled(
    U8G2_R2, //180 rotation
    10,      //cs
    9,       //dc
    8        //reset
);

int dataArray[64] = {0};
#define SHAKE_THRESHOLD 1000

void blinkOK()
{
  delay(1000);
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(13, !digitalRead(13));
    delay(200);
  }
}

void setup()
{
  Serial.begin();
  Wire.begin();
  oled.begin();
  oled.setFont(u8g2_font_t0_15b_mf);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Hello World!");
  u8g2.sendBuffer();

  if (!accel.begin())
  {
    Serial.println("Unable to start the accelerometer, check connections");
    Serial.println("and restart");
    while (1)
      ;
  }
}
void loop()
{
  for (int i = 0; i < 256;)
  {
    while (!accel.available())
      ;

    int val = accel.getX() + accel.getY() + accel.getZ();

    Serial.print("(");
    Serial.print(accel.getX());
    Serial.print(",");
    Serial.print(accel.getY());
    Serial.print(",");
    Serial.print(accel.getZ());
    Serial.print(") = ");
    Serial.println(val);

    if (val > SHAKE_THRESHOLD)
    {
      dataArray[i++] = val;
    }

    switch (i)
    {

    case 10:
      //keep shaking
      break;
    case 20:
      //keep shaking
      break;
    case 30:
      //keep shaking
      break;
    case 40:
      //keep shaking
      break;
    }
  }

  //we have a full array of "random" data
}