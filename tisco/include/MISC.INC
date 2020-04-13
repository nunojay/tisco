
  /*> misc.inc <*
   *
   * Date: 08/11/1998
   * Last Change: 08/11/1998
   *
   * Usefull types, macros, constants and routines. Generic.
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */



#ifndef __misc_inc__
#define __misc_inc__



#define NULL  0x0

#define TRUE     1
#define FALSE    0

typedef char  bool;



//*******************************
//** A few usefull asm macros...

#define LD_DE_HL    asm "    ld     e, l";  \
                    asm "    ld     d, h";
#define LD_HL_DE    asm "    ld     l, e";  \
                    asm "    ld     h, d";



//*******************************
//** A few usefull functions...

uint POPW ()
{
    asm {
    pop    bc
    pop    hl
    push   bc
    };
}

uchar POPB ()
{
    asm {
    pop    de
    pop    af
    push   de
    };
}


#endif
