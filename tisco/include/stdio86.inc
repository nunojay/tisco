
  /*> stdio86.inc <*
   *
   * Date: 20/10/1998
   * Last Change: 25/11/1998
   *
   * Basic IO/TISCO-suport rotines and external declarations for TI-86.
   * Needs ti86asm.inc and ti86undc.inc in assembly file.
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#ifndef __stdio86_inc__
#define __stdio86_inc__

#include "misc.inc"


/**************************************************************************
 ** MACROS, TYPES AND CONSTANTS                                          **/

// Exits (safetly) from program
#define exit    asm "    jp     0409Ch";     // _jforcecmdnochar (undoc)

#define NULL    0x0



// [NTBT: Forget this:]
// This struct is used for main parameters and with overlays
typedef struct regs_struc {
   uint    af;
   uint    bc;
   uint    de;
   uint    hl;
 }



// Some defines for readkey keys
#define kRIGHT      0x01
#define kLEFT       0x02
#define kUP         0x03
#define kDOWN       0x04
#define kENTER      0x06
#define kEXIT       0x07
#define kDEL        0x09
#define kF1         0xC2
#define kF2         0xC3
#define kF3         0xC4
#define kF4         0xC5
#define kF5         0xC6


#define TEXT_SHADOW      0xC0F9       // 168 bytes


#define SetNoScroll   asm "   res   appAutoScroll, (iy + appflags)"
#define SetScroll     asm "   set   appAutoScroll, (iy + appflags)"

/**************************************************************************
 ** EXTERNAL VARIABLES                                                   **/

extern  uchar curRow;
extern  uchar curCol;
extern  uchar penCol;
extern  uchar penRow;

extern  uchar winTop;
extern  uchar winBtm;

/**************************************************************************
 ** EXTERNAL FUNCTIONS (ROM FUNCTIONS)                                   **/

void clrLCD ();
void clrScrn ();
void cpyText ();         // copy text shadow to screen
uchar getkey ();

void newline ();
void homeup ();
void putc();

void runindicon ();
void runindicoff ();
void calcOff ();        // Turns calc off (undoc)
void pause ();          // Pauses calc as the BASIC command (undoc)
void cursorOn ();
void cursorOff ();

void dispOP1 ();        // Puts OP1 on screen (undoc)


/**************************************************************************
 ** FUNCTIONS                                                            **/


void enough_mem ();


bool check_enough_mem (uint size)
{
    getv  size;
    enough_mem();
    ifc  return FALSE;       // Not enough
    return TRUE;             // Enough
}



/*  NOT FUNCTIONAL
void call_asmprg (char *prgm_name, regs_struc *args, regs_struc *res)
// This function calls another asm program. args gets the values of
// the registers (not all; see type regs_struc) to pass to the called
// program, and res receives the returned register values.
// prgm_name is the name of the called program. Must have it's length
// in the second character, and the first is a space
// (Ex.: " \005Hello".
{
    getv prgm_name;                      // hl = prgm_name
    asm "      call    _MOV10TOOP1";     // put prgm name on OP1

    getv args;               // hl = args
   // Now load registers with values in args, call exec_assembly and
   // put returned registers in res.
    asm {
    ld      e, (hl)         ; af
    inc     hl
    ld      d, (hl)         ; af
    push    de
    inc     hl
    ld      c, (hl)         ; bc
    inc     hl
    ld      b, (hl)         ; bc
    inc     hl
    ld      e, (hl)         ; de
    inc     hl
    ld      d, (hl)         ; de
    inc     hl
    call    _ldhlind        ; hl = (hl)     (undoc, destroys A)
    pop     af

    call    _exec_assembly

    push    hl
    push    de
    push    bc
    push    af     };
    getv res;                // hl = res
    asm {
    pop     bc
    ld      (hl), c         ; af
    inc     hl
    ld      (hl), b         ; af
    inc     hl
    pop     bc
    ld      (hl), c         ; bc
    inc     hl
    ld      (hl), b         ; bc
    inc     hl
    pop     bc
    ld      (hl), c         ; de
    inc     hl
    ld      (hl), b         ; de
    inc     hl
    pop     bc
    ld      (hl), c         ; hl
    inc     hl
    ld      (hl), b               };
}
*/


void locate (uchar x, uchar y)
// Locates the screen cursor to (x,y) (fixed font width)
{
    curRow = y;
    curCol = x;
}

uchar putstr (char *st)
// Puts a 0-terminated string on current cursor location. (fixed font width)
{
    getv st;
    asm "    xor    a";
    asm "    call   _puts";
    ifnc  return TRUE;
}


void vlocate (uchar x, uchar y)
// Locates the screen pen to (x,y) (variable font width)
{
    penRow = y;
    penCol = x;
}

void setTextEraseBelow (bool state)
{
    asm "     res   textEraseBelow, (iy + textflags)";
    if (state)
       asm "     set   textEraseBelow, (iy + textflags)";
}

void vputstr (char *st)
// Puts a 0-terminated string on current pen location. (variable font width)
{
    getv  st;
    asm "    call   _vputs";
}


void putuint (uint val)
// Puts an 16bit unsigned value on screen, using the fixed width font
{
    getv  val;
    asm "    xor    a";
    asm "    call   _dispAHL";     // (undoc)
}


void SetUpperAlphaLock ()
{
    asm "    res    shiftLwrAlph, (iy + shiftflags)";
    asm "    set    shiftALock, (iy + shiftflags)";
    asm "    set    shiftAlpha, (iy + shiftflags)";
}


void SetLowerAlphaLock ()
{
    asm "    set    shiftAlpha, (iy + shiftflags)";
    asm "    set    shiftALock, (iy + shiftflags)";
    asm "    set    shiftLwrAlph, (iy + shiftflags)";
}


void UnlockAlphas ()
{
    asm "    res    shiftALock, (iy + shiftflags)";
    asm "    res    shiftAlpha, (iy + shiftflags)";
    asm "    res    shiftLwrAlph, (iy + shiftflags)";
}



#define __ConvTabSize      (2*8)
extern char get_str_ConvTab[__ConvTabSize];

uchar readkey ()
{
    asm {
     call  _getkey
     cp    kAdd
     ret   c        ; out if a < '+' (fn can't have args or locals,
     cp    kz+1     ;                 to ret like this)
     ret   nc       ; out if a > 'z'

     cp    kSpace
     jr    nz, no_space
     ld    a, ' '
     ret

no_space:
  ; here, we know a is between kAdd and kz
     ld    b, 0     ; add value for lookup table
     ld    d, b
     cp    k0
     jr    c, get_by_table

     ld    b, L0 - k0     ; add value for '0'..'9'
     cp    k9+1
     jr    c, out_with_add

     ld    b, LcapZ - kCapZ    ; add value for 'A'..'Z'
     cp    kCapZ+1
     jr    c, out_with_add

     add   a, La - ka          ; add value for 'a'..'z'
     ret

get_by_table:
     ld    hl, _get_str_ConvTab
     sub   kAdd
     ld    e, a
     add   hl, de
     ld    a, (hl)

out_with_add:
     add   a, b       ; Possibly make lower case from upper case
     ret

_get_str_ConvTab:
       .db '+', '-', '*', '/', ' ', '(', ')', '['
;          Add  Sub  Mul  Div  Expon L/RParen LBrack
       .db ']', '=', ' ',  ' ',  ',', ' ', ' ', '.'
;      RBrack Equal Store Recall Comma Ang Chs
    };
}



char   *st;

bool get_str (char x, char y, char *str, char max_char)
/* Gets a string, and stores a 0 at the end. max_char is the max
 * number of chars to accept in the string.
 * Returns TRUE if the string was accepted (Enter pressed). */
{
    char   ch, max_curCol;

    curRow = y;
    curCol = x;
    max_curCol = max_char + curCol;
    st = str;

    for (ch = 0; TRUE; )  {
        cursorOn();
        ch = readkey();
        asm {
         cp   6
         jr   z, getstr_loop_out
         cp   7
         jr   z, getstr_loop_out
        };
        cursorOff();

        if (ch == kDEL)  {
           if (curCol != x)  {
              curCol--;
              asm "     ld    a, 32";    // space
              putc();
              curCol--;
              st--;
           }
        }
        else if (curCol != max_curCol)  {
              *st = ch;
              st++;
              getv  ch;
              putc();
           }
    }
    asm "getstr_loop_out:";
    *st = 0;
    if (ch == kENTER)
       return TRUE;      // Return TRUE if ENTER used
    return FALSE;        // Return FALSE if EXIT used
}



#endif

