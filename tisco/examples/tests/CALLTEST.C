
#include "stdio86.inc"


void fill_vec (char *v, char n)
{
   for (; n >= 0; n--)  {
       *v = n;
       v++;
   }
}


void test_fill_vec (char *v, char n)
{
   for (; n >= 0; n--)  {
       if (*v != n)  {
          newline();
          putstr(" - ERROR - ");
          pause();
          return;
       }
       v++;
   }
   newline();
   putstr(" - VEC OK - ");  newline();
   pause();
}


regs_struc   rin, rout;

void main (regs_struc *args)
{
     char   v[110];

     clrLCD();
     locate(0,0);

     fill_vec(&v, 109);
     rin.af = 0xFF00;
     rin.bc = 0x0;
     rin.de = 0x1;
     rin.hl = 0x2;
     call_asmprg(" \006CALLED", &rin, &rout);

     clrLCD();
     locate(0,0);
     putstr("I am back");
     test_fill_vec(&v, 109);

     clrLCD();
     locate(0,0);
     putstr("Returned:");   newline();
     putstr(" AF:"); putuint(rout.af); newline();
     putstr(" BC:"); putuint(rout.bc); newline();
     putstr(" DE:"); putuint(rout.de); newline();
     putstr(" HL:"); putuint(rout.hl); newline();
     pause();
     newline();
     putstr(" T H E  E N D . . .");  newline();
}


