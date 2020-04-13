
  /*> math86.inc <*
   *
   * Date: 20/10/1998
   * Last Change: 08/11/1998
   *
   * OPx math rotines for TI-86.
   * Needs ti86ops.inc and ti86math.inc in assembly file.
   *
   * The routines here use only OP1-OP2 for calculations, and OP5-OP6
   * as temp registers.
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#ifndef __math86_inc__
#define __math86_inc__

#include "misc.inc"


void ckop1fp0();


void uint2OP1 (uint val)
// Load OP1 with 16bit value
{
     asm "    call   _OP2TOOP6";      // Save OP2
     getv val;
     asm {
     call   _SetXXXXOP2
     call   _OP2TOOP1
     call   _OP6TOOP2      ; Restore OP2
     };
}

void uint2OP2 (uint val)
// Load OP2 with 16bit value
{
     getv val;
     asm "    call   _SetXXXXOP2";
}

bool FPPOS ()
// Returns TRUE if OP1 >= 0
{
    ckop1fp0();
    ifz  return TRUE;
    else return FALSE;
}

void TIMESPT5 ();     // OP1 = OP1 / 2

void OP1TOOP2 ();     // OP2 = OP1
void OP2TOOP1 ();     // OP1 = OP2
void OP1TOOP5 ();     // OP5 = OP1
void OP5TOOP1 ();     // OP1 = OP5
void OP2TOOP6 ();     // OP6 = OP2
void OP6TOOP2 ();     // OP2 = OP6

void FPADD ();        // OP1 = OP2 + OP2
void FPSUB ();        // OP1 = OP2 - OP2
void FPMULT ();       // OP1 = OP2 * OP2
void FPDIV ();        // OP1 = OP2 / OP2

void FPSQUARE ();     // OP1 = sqr(OP1)
void SQROOT ();       // OP1 = sqrt(OP1)
void MINUS1 ();       // OP1--
void FPRECIP ();      // OP1 = 1 / OP1

void INTGR ();        // OP1 = int(OP1)
void FRAC ();         // OP1 = frac(OP1)
uchar CONVOP1 ();     // A = OP1

void SIN ();          // OP1 = sin(OP1)
void COS ();          // OP1 = cos(OP1)
void TAN ();          // OP1 = tan(OP1)

void FPROUND ()
// OP1 = round(OP1)
{
    OP1TOOP5();      // save OP1
    OP2TOOP6();      // save OP2
    if (FPPOS())  uint2OP1(1);    // OP1 = +-0.5
    else          uint2OP1(-1);
    TIMESPT5();      // OP1 = +-1 / 2 = +-0.5
    OP1TOOP2();      // OP2 = +-0.5
    OP5TOOP1();      // Restore OP1 (old OP1)
    FPADD();         // OP1 = old OP1 +- 0.5
    INTGR();         // OP1 = round(old OP1)
    OP6TOOP2();      // Restore OP2
}

void RANDOM ();


#endif
