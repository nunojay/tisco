
  /*> crt.c <*
   *
   * Date: 23/01/1999
   * Last Change: 23/01/1999
   *
   * Lib for easy writing of 'text mode' applications.
   *
   * Compile with
   *     ..\>tisco CRT.C -d -jo
   *
   * By NSJ aka Viriato 1999
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#include "crt.h"



/**************************************************************************/
/** DLL data                                                             **/

bool clip;       // Clip on or off
bool line_wrap;  // Line wrap on or off

char vp_xMin, vp_yMin,     // Current viewport
     vp_xMax, vp_yMax;

char stack_vpxMin, stack_vpyMin,     // The viewport stack
     stack_vpxMax, stack_vpyMax;


extern uchar curRow;       // Current cursor location (or just 'location')
extern uchar curCol;

// Current cursor location stack. There's one more for internal use.
uchar stack_curRow[LOCATION_STACK_SIZE+1];
uchar stack_curCol[LOCATION_STACK_SIZE+1];
uchar *xlocation_TOS;       // Top of location stack
uchar *ylocation_TOS;



extern char *textShadow;   // Used as shadow screen


/**************************************************************************/
/** DLL Internal Functions                                               **/

void clrScrn ();
void cursorOn ();
void cursorOff ();
void putc ();
char getkey ();

/*
void NextLoc ()
// Advances the cursor one position inside the viewport.
{
     curCol++;
     if (line_wrap)
       if (curCol > vp_xMax)  {
          curCol = vp_xMin;
          if (curRow <= vp_yMax)
             curRow++;
       }
}
/**/

bool Outside ()
// Returns != 0 iif the current cursor location is outside the viewport
{
     if (vp_xMin > curCol) return 1;
     if (curCol > vp_xMax) return 1;
     if (vp_yMin > curRow) return 1;
     if (curRow > vp_yMax) return 1;
     return 0;
}


/**************************************************************************/
/** DLL Exported Functions                                               **/


uint CRT () export 0
// Returns the DLL version: 0.0
{
    return 0*256+0;        // Version 0.0
}


void initCRT () export 1
// Initializes the CRT DLL.
// The Viewport is the entire screen, clipping and line wraping are ON
// and cursor OFF. The screen isn't cleared. The location ins't set.
{
     // No auto scroll
     asm "  res   appAutoScroll, (iy + appflags)";
     // Write also to the textShadow
     asm "  res   appTextSave, (iy + appflags)";
     // No text inverted
     asm "  res   textInverse, (iy + textflags)";

     // Viewport as the entire screen
     vp_xMin = 0;
     vp_yMin = 0;
     vp_xMax = crtXRes - 1;
     vp_yMax = crtYRes - 1;

     clip = TRUE;
     line_wrap = TRUE;
     cursorOff();

     // Init location stack
     xlocation_TOS = &stack_curCol;
     ylocation_TOS = &stack_curRow;
}


uchar getErrorCode () export 2
{}


void setClip (bool state) export 3
// Sets cliping ON or OFF
{
     clip = state;
}

void setLineWrap (bool state) export 4
// Sets line wrapping ON or OFF
{
     line_wrap = state;
}


void gotoxy (char x, char y) export 5
// Sets the current cursor location
{
     curCol = x + vp_xMin;
     curRow = y + vp_yMin;
}


void pushLocation () export 6
// Save the current cursor location in the stack
{
     *xlocation_TOS = curCol;
     xlocation_TOS++;
     *ylocation_TOS = curRow;
     ylocation_TOS++;
}

void popLocation () export 7
// Get the current cursor location from the stack
{
     curCol = *xlocation_TOS;
     xlocation_TOS--;
     curRow = *ylocation_TOS;
     ylocation_TOS--;
}


void cursorON () export 8
// Turns the cursor ON
{
     cursorOn();
}

void cursorOFF () export 9
// Turns the cursor OFF
{
     cursorOff();
}


void putChar (char ch) export 10
// Puts a character on the screen, at the current cursor location
// and advances the current cursor location. If clipping is on it
// performs cliping.
// This function is used by all the other functions that write to
// the screen, so all functions can have clipping.
{
     if (clip)
        if (Outside())
           return;       // Return if clip on and out of the viewport

     getv  ch;
     putc();
     if (line_wrap)
        if (curCol > vp_xMax)  {
           curCol = vp_xMin;
           if (curRow <= vp_yMax)
              curRow++;
        }
}


char getChar (char x, char y) export 11
// Returns the screen character at the location given
{
     char  *ofs;
     ofs = y;
     ofs = ofs * crtXRes;
     ofs = ofs + textShadow + x;
     return *ofs;
}


void clrScr () export 12
// Clears the entire screen and sets the Viewport as the entire screen
{
     clrScrn();
     vp_xMin = 0;
     vp_yMin = 0;
     vp_xMax = crtXRes - 1;
     vp_yMax = crtYRes - 1;
}


void clrViewport () export 13
// Clears the Viewport
{
     uchar   y;
     for (y = vp_yMin; y <= vp_yMax; y++)
         clrLine(y);
}

void clrLine (char y) export 14
// Clears line y
{
     char  x;
     pushLocation();
     curCol = vp_xMin;
     curRow = vp_xMax;
     for (x = vp_xMin; x <= vp_xMax; x++)
         putChar(' ');
     popLocation();
}



void delLine (char y) export 15
// Deletes a line at line y. All lines below line y
// are shifted up by one line, leaving the bottom line repeated.
{
}

void insLine (char y) export 16
// Inserts an line at line y. Line y and all the lines below
// are shifted down by one line. Line y gets repeated at line y+1
{
}


void setViewport (char x1, char y1, char x2, char y2) export 17
// Sets the Viewport. All other drawing function's coordinates
// are relative to the Viewport's top left coorner.
{
     vp_xMin = x1;
     vp_yMin = y1;
     vp_xMax = x2;
     vp_yMax = y2;
}


char getViewportWidth () export 18
// Returns the viewport width
{
     return vp_xMax - vp_xMin + 1;
}

char getViewportHeight () export 19
// Returns the viewport height
{
     return vp_yMax - vp_yMin + 1;
}


void pushViewport () export 20
// Save the Viewport limits from the viewport stack
{
     stack_vpxMin = vp_xMin;
     stack_vpyMin = vp_yMin;
     stack_vpxMax = vp_xMax;
     stack_vpyMax = vp_yMax;
}

void popViewport () export 21
// Get the Viewport limits from the viewport stack
{
     vp_xMin = stack_vpxMin;
     vp_yMin = stack_vpyMin;
     vp_xMax = stack_vpxMax;
     vp_yMax = stack_vpyMax;
}


void frame (char *title, uchar flags) export 22
// Draws a border around the viewport. The area of the viewport
// itself is untouched. title has the frame's title or NULL for no
// title. Pass 0 for flags, I'm still thinking of what to put there.
{
}


void scrollViewport (char dx, char dy) export 23
// Scrolls the current viewport dx units horizontally and dy units
// vertically. dx and dy can be negative, meaning up and left.
{
}


void getArea (char *data) export 24
// Gets the contents of the viewport and saves it in a buffer
// pointed to by data. The size must be (width*heigth+2).
{
}

void putArea (char x, char y, char *data) export 25
// Puts a previously saved area (saved with getArea) into the
// screen, at the position given.
{
}


//////////////////////////////////////////////////////////////////////
// Data puting functions

void putFloat (char max_decimal) export 26
// Puts OP1 at the current cursor location, puting at most max_decimal
// decimal places (putFloat(3) of OP1=1.12345 will put 1.123). NOTE
// that the number is trunced. Use define ALL to put the entire number.
{
}


void putInt (int i) export 27
// Puts an signed int at the current cursor location
{
}


void putUInt (uint u) export 28
// Puts an unsigned int at the current cursor location
{
}


void putHex (uint h) export 29
// Puts an unsigned int at the current cursor location in hexadecimal
{
}


void putStr (char *str) export 30
// Puts a string at current cursor location
{
     for (; *str; str++)
         putChar(*str);
}


// Aux constants for getKey()
#define LcapZ      0x05A
#define L0         0x030
#define La         0x061
#define kAdd       0x00C
#define kSub       0x00D
#define kMul       0x00E
#define kDiv       0x00F
#define kLParen    0x011
#define kRParen    0x012
#define kLBrack    0x013
#define kRBrack    0x014
#define kEqual     0x015
#define kComma     0x018
#define kDecPnt    0x01B
#define kSpace     0x027
#define k0         0x1C
#define k9         0x25
#define kCapZ      0x41
#define ka         0x42


char  gch;

char getKey () export 31
// Waits for a key and returns it. Most codes it returns are ASCII codes.
{
    gch = getkey();
    getv  gch;
    asm {
     cp    kAdd
     ret   c        ; out if a < '+' (fn can't have args or locals,
     cp    kz+1     ;                 to ret like this)
     ret   nc       ; out if a > 'z'
    };

    // here, we know ch is between kAdd and kz
    if (k0 > gch)
       switch (gch)  {
           case kAdd : return '+';
           case kSub : return '-';
           case kMul : return '*';
           case kDiv : return '/';
           //case kExpon : return 'e';
           case kLParen : return '(';
           case kRParen : return ')';
           case kLBrack : return '[';
           case kRBrack : return ']';
           case kEqual : return '=';
           case kComma : return ',';
           case kDecPnt : return '.';
           //case kStore : case kRecall : case kAng : case kChs :
           default : return ' ';
       }

    if (gch == kSpace)
       return ' ';
    if (k9+1 > gch)
       return gch + (L0 - k0);        // add value for '0'..'9'
    if (kCapZ+1 > gch)
       return gch + (LcapZ - kCapZ);  // add value for 'A'..'Z'

    return gch + (La - ka);           // add value for 'a'..'z'
}


char   *st;    // Aux for getStr

bool getStr (char *str, char max_char) export 32
// Gets a string, and stores a 0 at the end. max_char is the max
// number of chars to accept in the string (excluding final '\0').
// Returns TRUE if the string was accepted (Enter pressed), else
// return FALSE (EXIT/QUIT pressed).
{
    char   ch, max_curCol, x;

    x = curCol;
    max_curCol = max_char + curCol;
    st = str;

    for (ch = 0; TRUE; )  {
        cursorOn();
        ch = getKey();
        if (ch == kENTER)
           return TRUE;
        if (ch == kEXIT)
           return FALSE;
        cursorOff();

        if (ch == kDEL)  {
           if (curCol != x)  {
              curCol--;
              putChar(' ');    // Del the char
              curCol--;
              st--;
           }
        }
        else if (curCol != max_curCol)  {
           *st = ch;
           st++;
           putChar(ch);
        }
        *st = 0;
    }

/*    if (ch == kENTER)
       return TRUE;      // Return TRUE if ENTER used
    return FALSE;        // Return FALSE if EXIT used
 /**/
}



