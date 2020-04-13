
  /*> crt.h <*
   *
   * Date: 23/01/1999
   * Last Change: 23/01/1999
   *
   * Lib for easy writing of 'text mode' applications.
   *
   * By NSJ aka Viriato 1999
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */

  /*
   * This unit allows screen size independant 'text mode applications'.
   * You can output characters, text and numbers easelly, with or
   * without clipping to a user defined viewport.
   * All operations take place within a viewport that you set (unless
   * clipping is off). The coordinates are all relative to the viewport's
   * top left corner (we call call it a viewport).
   * Every character writen in the screen advances the current cursor's
   * location.
   */



#ifndef __crt_h__
#define __crt_h__


#include "misc.inc"       // for bool, TRUE, FALSE, NULL, ...


// Screen dimensions (TI-86 sizes)
#define crtXRes   21
#define crtYRes   8


// Coordinate's Stacks sizes.
#define LOCATION_STACK_SIZE   1    // Max number of consecutive pushLocation
#define VIEWPORT_STACK_SIZE   1    // Max number of consecutive pushViewport


// Some defines for getKey keys
#define kRIGHT      0x01
#define kLEFT       0x02
#define kUP         0x03
#define kDOWN       0x04
#define kENTER      0x06
#define kEXIT       0x07
#define kCLEAR      0x08
#define kDEL        0x09
#define kINS        0x0A
#define kNEXT       0x0B
#define kF1         0xC2
#define kF2         0xC3
#define kF3         0xC4
#define kF4         0xC5
#define kF5         0xC6



dll CRT {
 uint CRT () export 0;
 void initCRT () export 1;

 uchar getErrorCode () export 2;

 void setClip (bool state) export 3;
 void setLineWrap (bool state) export 4;
 void gotoxy (char x, char y) export 5;
 void pushLocation () export 6;
 void popLocation () export 7;
 void cursorON () export 8;
 void cursorOFF () export 9;

 void putChar (char ch) export 10;
 char getChar (char x, char y) export 11;

 void clrScr () export 12;
 void clrViewport () export 13;
 void clrLine (char y) export 14;

 void delLine (char y) export 15;
 void insLine (char y) export 16;

 void setViewport (char x1, char y1, char x2, char y2) export 17;
 char getViewportWidth () export 18;
 char getViewportHeight () export 19;
 void pushViewport () export 20;
 void popViewport () export 21;
 void frame (char *title, uchar flags) export 22;
 void scrollViewport (char dx, char dy) export 23;
 void getArea (char *data) export 24;
 void putArea (char x, char y, char *data) export 25;

 void putFloat (char max_decimal) export 26;
 void putInt (int i) export 27;
 void putUInt (uint u) export 28;
 void putHex (uint h) export 29;
 void putStr (char *str) export 30;

 char getKey () export 31;
 bool getStr (char *str, char max_char) export 32;
}


#endif


