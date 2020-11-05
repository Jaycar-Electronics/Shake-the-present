
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <U8g2lib.h>
#include <SPI.h>

#include "santaho.h"
#include "shake.h"

#define DEBUGGING_INFO

static const char *const revelations[] PROGMEM = {
    "A pony",
    "A new car",
    "A left boot",
    "Two left boots",
    "A Kitten",
    "A duck in a box",
    "A keyboard without the letter E",
    "Triangular Bike wheels",
    "an invisible box",
    "A can of worms",
    "A really big microscope",
    "A dictionary dressed up like cake"
    };

// -----------------------------
const int shake_threshold = 1400;
const int mag_history_max = 16;
// -----------------------------

//void (*resetSelf)(void) = 0;

// global variables:
int magnitude_history[mag_history_max] = {0};
int mag_index = 0;
int shake_counter = 0;
bool tripped = false;
MMA8452Q accel;
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
    U8G2_R0, //180 rotation
    9,       //cs
    10,      //dc
    12       //reset
);
int rundown = 0;
long timer = 0;
enum state
{
  start,
  keep_shaking,
  reveal
};

state current_state = start;
state old_state = reveal; // just to trip the first check

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  oled.begin();
  oled.setFont(u8g2_font_t0_15b_mf);
  oled.setFontDirection(0);

  if (!accel.begin())
  {
    Serial.println("Unable to start the accelerometer, check connections");
    Serial.println("and restart");
    while (1)
      ;
  }
  Serial.println("trip, shake, mag, normalised, average");
}

void draw_santa()
{
  oled.drawXBMP(0, 0, santaho_width, santaho_height, santaho_bits);
}
void draw_shake()
{
  //oled.drawStr(0,10, "Keep shaking");
  oled.drawXBMP(0, 0, shake_width, shake_height, shake_bits);
}
void draw_reveal(int indx)
{
  oled.drawStr(0,10, "You are getting:");
  oled.drawStr(0, 30, revelations[indx]);
}
void loop()
{

  if (current_state != old_state && current_state != reveal)
  {
    oled.firstPage();
    Serial.println("Starting first page");
    do
    {
      if (current_state == start)
      {
        draw_santa();
      }
      else if (current_state == keep_shaking)
      {
        draw_shake();
        // tumble into what the person is getting
      }
      else
      {
        int r = random(0,5); //todo

        draw_reveal(r);

        // you are getting X;
      }

    } while (oled.nextPage());
    Serial.println("ending first page");

    old_state = current_state;
  }

  if (current_state == reveal)
  {
    Serial.println("Reveal!");
    delay(500);
    rundown--;
    if (rundown == 0)
    {
      //last one
      delay(5000); //5 seconds.
      //resetSelf();
    }
    return;
  }

  //this is code while it's detecting shakes

  while (!accel.available())
    ;

  int magnitude = sqrt(
      pow(accel.getX(), 2) +
      pow(accel.getY(), 2) +
      pow(accel.getZ(), 2));

  int avg = getAverageMagnitude(magnitude);

  if ((avg) > shake_threshold && !tripped)
  {
    tripped = true;
    shake_counter++;
  }
  else if ((avg) < shake_threshold && tripped)
  {
    tripped = false;
  }

  if (shake_counter < 5)
  {
    current_state = start;
  }
  else if (shake_counter < 10)
  {
    current_state = keep_shaking;
  }
  else
  {
    rundown = 6;
    current_state = reveal;
  }

#ifdef DEBUGGING_INFO
  Serial.print(shake_threshold);
  Serial.print(",");
  Serial.print(shake_counter * 100);
  Serial.print(",");
  Serial.print(magnitude);
  Serial.print(",");
  Serial.println(avg);
#endif
}

int getAverageMagnitude(int mag)
{
  magnitude_history[mag_index++] = mag;
  mag_index %= mag_history_max;
  int sum = 0;
  for (int i = 0; i < mag_history_max; i++)
  {
    sum += magnitude_history[i];
  }
  return sum / mag_history_max;
}

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