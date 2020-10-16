#include <Wire.h>
#include <paj7620.h>

#define GES_REACTION_TIME 500
#define GES_ENTRY_TIME 800
#define GES_QUIT_TIME 1000

#define OUTPIN A1
#define SHORTPULSE 316
#define LONGPULSE 818
const byte rfcmds[] = {0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x7, 0x6, 0x4, 0x8}; //nocmd, 1 on/off,2 on/off,3 on/off,4 on/off,all on/off
const unsigned long address = 0x12340;

void blinkOK()
{
  delay(1000);
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (int i = 0; i < 4; i++){
    digitalWrite(13, !digitalRead(13));
    delay(200);
  }
}
void setup()
{
  blinkOK();

  pinMode(A1, OUTPUT);
  uint8_t error = 0;
  Serial.begin(9600);
  Serial.println("Starting Program");

  error = paj7620Init(); // initialize Paj7620 registers
  if (error)
  {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  }
  else
  {
    Serial.println("INIT OK");
  }
}

void loop()
{
  uint8_t data = 0, data1 = 0, error;

  error = paj7620ReadReg(0x43, 1, &data); // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (!error)
  {
    switch (data) // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    {
    case GES_RIGHT_FLAG:
      delay(GES_ENTRY_TIME);
      Serial.println("Right");
      turnOnLight();
      break;
    case GES_LEFT_FLAG:
      delay(GES_ENTRY_TIME);
      Serial.println("Left");
      turnOffLight();
      break;
    }
    delay(100);
  }
}

void turnOffLight()
{
  Serial.println("    Sending ON packet to powerpoint");
  sendrf(packet(address, rfcmds[1])); //1 off
}
void turnOnLight()
{
  Serial.println("    Sending OFF packet to powerpoint");
  sendrf(packet(address, rfcmds[0])); //1 on
}
unsigned long packet(unsigned long a, byte c)
{ //takes address, command, calculates crc and sends it
  unsigned long p;
  byte cx;
  p = ((a & 0xFFFFF) << 4) | (c & 0xF);
  cx = rfcrc(p);
  p = (p << 8) | cx;
  return p;
}

byte rfcrc(unsigned long d)
{ //calculate crc
  byte a, b, c;
  a = reverse(d >> 16);
  b = reverse(d >> 8);
  c = reverse(d);
  return reverse(a + b + c);
}

byte reverse(byte d)
{ //reverse bit order in byte
  return ((d & 0x80) >> 7) | ((d & 0x40) >> 5) | ((d & 0x20) >> 3) | ((d & 0x10) >> 1) | ((d & 0x08) << 1) | ((d & 0x04) << 3) | ((d & 0x02) << 5) | ((d & 0x01) << 7);
}

void sendrf(unsigned long int k)
{ //send a raw packet
  unsigned long int i;
  for (int r = 0; r < 20; r++)
  { //do repeats-- do more if success rate is low
    for (i = 0x80000000UL; i > 0; i = i >> 1)
    { //32 bits of sequence
      if (i & k)
      { //hi bit
        digitalWrite(OUTPIN, HIGH);
        delayMicroseconds(LONGPULSE);
        digitalWrite(OUTPIN, LOW);
        delayMicroseconds(SHORTPULSE);
      }
      else
      { //lo bit
        digitalWrite(OUTPIN, HIGH);
        delayMicroseconds(SHORTPULSE);
        digitalWrite(OUTPIN, LOW);
        delayMicroseconds(LONGPULSE);
      }
    }
    digitalWrite(OUTPIN, HIGH); //3 more lo bits
    delayMicroseconds(SHORTPULSE);
    digitalWrite(OUTPIN, LOW);
    delayMicroseconds(LONGPULSE);
    digitalWrite(OUTPIN, HIGH);
    delayMicroseconds(SHORTPULSE);
    digitalWrite(OUTPIN, LOW);
    delayMicroseconds(LONGPULSE);
    digitalWrite(OUTPIN, HIGH);
    delayMicroseconds(SHORTPULSE);
    digitalWrite(OUTPIN, LOW);
    delayMicroseconds(LONGPULSE);
    delayMicroseconds(8000); //brief delay between repeats
  }
}