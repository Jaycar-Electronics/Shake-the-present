#define DEBUGGING_INFO

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include "definitions.h"
#include "santa_ho_image.h"
#include "shake.h"
#include "present_strings.h"
/**************
* Constants
* and other defines
*
*
*
*/


const uint8_t mag_history_max = 16;
const short shake_threshold = 1400;


void (*resetSelf)(void) = 0;

enum GameState {
    initial,
    keep_shaking,
    reveal
};

int measureShake();
short getAverageMagnitude(int);
void draw(GameState state);
/**************
* Global vars
*
*
*/

uint8_t shake_counter = 0; //count how many times we've shaken it
uint8_t present_rundown = 0;        // number of presents to loop through
short magnitude_history[mag_history_max] = {0};
uint8_t mag_idx = 0;

bool tripped = false; //have we reached high accel?
bool dirty = true;  //do we need to re-draw the screen?

uint8_t lastShake = 0; // for debugging;

GameState state = initial;


/**************
* Setup and Loop
* Main routines
*
*/


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  randomSeed(analogRead(0));
}

void loop()
{

  if (dirty && state != reveal)
  {
    draw(state);
    dirty = false;
  }

  else if (state == reveal){
    draw_reveal();
    return;
  }

  //we are not revealing, count shakes:
  short shake = measureShake();

  //------- check shakes ----------------
  if (shake > shake_threshold && !tripped){
    tripped = true;
    shake_counter++;
    Serial.println("Tripped!");
  }
  else if (shake < shake_threshold && tripped){
    tripped = false;
  }
  //------------------------------------


  //------- check state ----------------
  if(shake_counter > 10 && state != reveal){
    state = reveal;
    dirty = true;
  } else if (shake_counter > 5 && state != keep_shaking){
    state = keep_shaking;
    dirty = true;
  }
  //------------------------------------
  //now it will run the loop again, draw if needed.
  lastShake = max(shake, lastShake);

  if((shake+100) < lastShake && lastShake > 0){
    Serial.print("Shake Max: ");
    Serial.print(lastShake);
    Serial.print(" Thresh: ");
    Serial.print(shake_threshold);
    Serial.print(" ShakeCount:");
    Serial.println(shake_counter);
    lastShake = 0;
  }
}


/**************
* Private functions for
* Accelerometer and OLED
*
*/

void draw_reveal(){
  U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
      U8G2_R0, //180 rotation
      9,       //cs
      10,      //dc
      12       //reset
  );

  oled.begin();
  oled.setFont(u8g2_font_t0_11_me);
  oled.setFontDirection(0);

  //todo random

  char buffer[20];
  uint8_t x = random(0, presents_count);
  for(uint8_t i = 0; i < 20; i++){


    uint8_t val = (x+i) % presents_count;

    char * ptr = (char*) pgm_read_word(
      &(presents [val])
      );

    strcpy_P(buffer, ptr);


    oled.firstPage();
    do{
        oled.drawStr(30, 10, "You will get:");

        uint8_t x = 0;
        uint8_t y = 33;
        oled.drawHLine(0, 17, 128);
        oled.drawHLine(0, 20, 128);
        for(uint8_t c = 0; buffer[c] != 0; c++){
          if(buffer[c]=='\n'){
            x = 5; y = 45;
          }
          oled.setCursor(x,y);
          oled.print(buffer[c]);
          x+=6;
        }
        oled.drawHLine(0, 50, 128);
        oled.drawHLine(0, 52, 128);
        //oled.print(buffer);
    } while (oled.nextPage());
    float frac = i / 6;

    delay(
      100 * pow(frac, 2)
    );

  }


  //at the end of the function,
  // oled is removed and memory is restored
  delay(10000L);
  resetSelf();
}


/*
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
    U8G2_R0, //180 rotation
    9,       //cs
    10,      //dc
    12       //reset
);*/

void draw(GameState state){

  U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(
      U8G2_R0, //180 rotation
      9,       //cs
      10,      //dc
      12       //reset
  );

  oled.begin();
  oled.setFont(u8g2_font_t0_15b_mf);
  oled.setFontDirection(0);

  //todo random

  oled.firstPage();
  do{
    if (state == initial){
      oled.drawXBMP(0, 0, 128, 64, santa_ho_image);
    }
    else if ( state == keep_shaking) {
      //oled.drawStr(20, 40, "Keep shaking");
      oled.drawXBMP(0, 0, shake_width, shake_height, shake_bits);
    }
    else {
      oled.drawStr(0, 10, "You won");
      //draw_reveal();
    }
  } while (oled.nextPage());

  //at the end of the function,
  // oled is removed and memory is restored
}




short getAverageMagnitude(int mag)
{
  magnitude_history[mag_idx++] = mag;
  mag_idx %= mag_history_max;
  short sum = 0;
  for (uint8_t i = 0; i < mag_history_max; i++)
  {
    sum += magnitude_history[i];
  }
  return sum / mag_history_max;
}


int measureShake(){
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

  short v = getAverageMagnitude(sqrt(
      pow(accel.getX(), 2) +
      pow(accel.getY(), 2) +
      pow(accel.getZ(), 2)));

  return v;

  //after this point the accel is removed from memory
}

