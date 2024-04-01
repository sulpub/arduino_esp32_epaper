/*
  Barcode.h - Creates a Barcode (Code 128) from a string
  Created by Victor Heid Kunamitsu Mikó
  Released into the public domain.
*/
#ifndef Barcode_h
#define Barcode_h

#include "Arduino.h"


class Barcode
{
  public:
    Barcode();
    // Returns a String containing the numbers representing the length of each bar, alternating between Black / White
    String make(String);
    String get_bars(char, int&, int);
};

#endif