// Libraries
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>

//Strings to show on the screen
#include "present_strings.h"

// Images generated with GIMP.
#include "santa_image.h"
#include "shake_image.h"
// Have a look at the readme file for how to build the bitmap images.

//#define RAW_LOG

/*************** Constants *****************************************
* and other defines
*/

const uint8_t mag_history_max = 16; //window of averaging function
const short shake_threshold = 1400; //the threshold that the shake must overcome

void (*resetSelf)(void) = 0; //shortcut to reset the arduino.

enum GameState
{ //internal state tracking. (State Machine)
  initial,
  keep_shaking,
  reveal
};

//function definitions:
int measureShake();             //get the shake magnitured
short getAverageMagnitude(int); //get average shake, without recording new values
void draw(GameState state);     // draw images on the screen.

/*************** Global vars ******************************************
*/

uint8_t shake_counter = 0;                      //count how many times we've shaken it
short magnitude_history[mag_history_max] = {0}; //the average window function as above
uint8_t mag_idx = 0;                            //internal counter to spots in the mag_history
int raw_mag = 0;

bool tripped = false; //have we reached high accel?
bool dirty = true;    //do we need to re-draw the screen?

uint8_t lastShake = 0; //for debugging;

GameState state = initial; //start at the initial state

/*************** Setup and Loop ********************************************
* Main routines
*/

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  randomSeed(analogRead(0));
#ifdef RAW_LOG
  Serial.println("raw, avg, thresh, count");
#endif
}

void loop()
{

  if (dirty && state != reveal)
  {
    draw(state);
    dirty = false;
  }

  else if (state == reveal)
  {
    draw_reveal();
    return;
  }

  //we are not revealing, count shakes:
  short shake = measureShake();

  //------- check shakes ----------------
  if (shake > shake_threshold && !tripped)
  {
    tripped = true;
    shake_counter++;
    Serial.println("Tripped!");
  }
  else if (shake < shake_threshold && tripped)
  {
    tripped = false;
  }
  //------------------------------------

  //------- check state ----------------
  if (shake_counter > 10 && state != reveal)
  {
    state = reveal;
    dirty = true;
  }
  else if (shake_counter > 5 && state != keep_shaking)
  {
    state = keep_shaking;
    dirty = true;
  }
  //------------------------------------
  //now it will run the loop again, draw if needed.

  // This is just debugging to help without
  // spamming the serial monitor. If you want

#ifndef RAW_LOG
  lastShake = max(shake, lastShake);

  if ((shake + 100) < lastShake && lastShake > 0)
  {
    Serial.print("Shake Max: ");
    Serial.print(lastShake);
    Serial.print(" Thresh: ");
    Serial.print(shake_threshold);
    Serial.print(" ShakeCount:");
    Serial.println(shake_counter);
    lastShake = 0;
  }

#else
  Serial.print(raw_mag);
  Serial.print(",");
  Serial.print(shake);
  Serial.print(",");
  Serial.print(shake_threshold);
  Serial.print(",");
  Serial.print(shake_counter * 100);
  Serial.println();
#endif
}

/*************** Private functions *******************************************
* Accelerometer and OLED
*/

void draw(GameState state)
{
  /* We use this function
  * To create the OLED management object
  * and draw on the screen.
  * This is so when the function ends, the oled
  * object will be deleted and the memory will be freed.
  * This greatly helps with crashing
  * As this program is very close to maximum practical use on the arduino
  * before crashing.
  */

  U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
      U8G2_R0, //180 rotation
      9,       //cs
      10,      //dc
      12       //reset
  );

  oled.begin();
  // keep in mind, we're not actually using text here
  // so we don't have to load the fonts.

  //oled.setFont(u8g2_font_t0_15b_mf);
  //oled.setFontDirection(0);
  oled.firstPage();

  do
  {

    if (state == initial)
    {
      oled.drawXBMP(0, 0, 128, 64, santa_image_bits);
    }
    else if (state == keep_shaking)
    {
      //oled.drawStr(20, 40, "Keep shaking");
      oled.drawXBMP(0, 0, 128, 64, shake_image_bits);
    }
  } while (oled.nextPage());
}

void draw_reveal()
{
  /* This function is very similar to the above
  *  Except with the random "dice roll" effect
  *  to show what the user is getting.

  * This *could* be refactored nicer,
  * But as long as it works, it works.
  */

  U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
      U8G2_R0, //180 rotation
      9,       //cs
      10,      //dc
      12       //reset
  );

  oled.begin();
  oled.setFont(u8g2_font_t0_11_me); //now we are using fnot
  oled.setFontDirection(0);

  char buffer[20];
  uint8_t x = random(0, presents_count); //randomly pick from the entire range

  for (uint8_t i = 0; i < 20; i++)
  {

    uint8_t val = (x + i) % presents_count; //scroll though

    //get a pointer to the string at this position
    char *ptr = (char *)pgm_read_word(&(presents[val]));

    //copy it into the buffer
    strcpy_P(buffer, ptr);

    oled.firstPage();
    do
    {
      oled.drawStr(30, 10, "You will get:");
      uint8_t x = 0;
      uint8_t y = 33;
      oled.drawHLine(0, 17, 128);
      oled.drawHLine(0, 20, 128);

      for (uint8_t c = 0; buffer[c] != 0; c++)
      {
        // we want to draw letter by letter so that we can handle newlines
        // easier.

        if (buffer[c] == '\n')
        {
          x = 5;
          y = 45;
        }

        oled.setCursor(x, y);
        oled.print(buffer[c]);

        x += 6; //one letter width
      }
      oled.drawHLine(0, 50, 128);
      oled.drawHLine(0, 52, 128);
    } while (oled.nextPage());

    //a semi nice exponential function to slow down dice roll.
    // really happy for someone to do a nicer "slow roullette"
    // effect
    // given it's a microcontroller, don't ask for too much.
    float frac = i / 6;
    delay(100 * pow(frac, 2));
  }

  //we finally "rest" on a position for 10 seconds, to show the user
  // then reset.
  delay(10000L);
  resetSelf();
}

short getAverageMagnitude(int mag)
{
  //general rolling window-esqe type of thing.
  magnitude_history[mag_idx++] = mag;
  mag_idx %= mag_history_max;
  short sum = 0;
  for (uint8_t i = 0; i < mag_history_max; i++)
  {
    sum += magnitude_history[i];
  }
  return sum / mag_history_max;
}

int measureShake()
{
  MMA8452Q accel;
  if (!accel.begin())
  {
    Serial.println(F("Unable to start the accelerometer, check connections"));
    Serial.println(F("and restart"));
    while (1)
      ;
  }

  while (!accel.available())
    ;

  raw_mag = sqrt(
      pow(accel.getX(), 2) +
      pow(accel.getY(), 2) +
      pow(accel.getZ(), 2));

  short v = getAverageMagnitude(raw_mag)

      return v;

  //after this point the accel is removed from memory
}
