#ifndef PRESENT_H__
#define PRESENT_H__

const uint8_t presents_count  = 13;

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