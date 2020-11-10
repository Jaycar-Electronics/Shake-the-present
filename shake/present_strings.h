#ifndef _PRESENT_H__
#define _PRESENT_H__

/*  This must be the number of messages as defined below; otherwise you
    won't be able to see all messages
*/
const uint8_t presents_count  = 13;

/*
We  *must* define each string as individually PROGMEM'd arrays.
This is just a fault with the AVR/GCC/Arduino system
*/

const char message00 [] PROGMEM = "A dictionary dressed\nup like cake";
const char message01 [] PROGMEM = "Half of a single\ncheeto";
const char message02 [] PROGMEM = "A pony!!\n check under pillow!";
const char message03 [] PROGMEM = "A new car";
const char message04 [] PROGMEM = "A left boot";
const char message05 [] PROGMEM = "Two left boots";
const char message06 [] PROGMEM = "A Kitten";
const char message07 [] PROGMEM = "The chicken and egg\nproblem, sans egg";
const char message08 [] PROGMEM = "A keyboard without\nthe letter E";
const char message09 [] PROGMEM = "Triangular\nBike wheels";
const char message10 [] PROGMEM = "An invisible box";
const char message11 [] PROGMEM = "A can of worms";
const char message12 [] PROGMEM = "A comically large\n microscope";

/*
Once we have defined each of the messages we can put them in the array like such
and PROGMEM the entire array. This works out pretty well in terms of space saving.

Keep the messages in order here, so that you can quickly check what messages are missing
the randomisation is put in the program code
*/
const char * const presents[presents_count] PROGMEM =
{
message00,
message01,
message02,
message03,
message04,
message05,
message06,
message07,
message08,
message09,
message10,
message11,
message12
};

#endif