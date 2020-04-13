
#include "stdio86.inc"
#include "math86.inc"


void spoil_vec (char *v, char n)
{
   for (; n > 0; n--)  {
       *v = 0;
       v++;
   }
}


void main (regs_struc *args)
{
     char   v[120];

     clrLCD();
     locate(0,0);
     putstr("I'm in CALLED");
     newline();  newline();

     locate(0,1);
     putstr("Main args:");
     newline();
     putstr(" AF:"); putuint(args->af); newline();
     putstr(" BC:"); putuint(args->bc); newline();
     putstr(" DE:"); putuint(args->de); newline();
     putstr(" HL:"); putuint(args->hl); newline();
     pause();

     spoil_vec(&v, 119);
     asm {
       ld    a, 0FFh;
       ld    bc, 2
       ld    de, 1
       ld    hl, 0
;       ret         ; so that the fini code of main doesn't destroy DE
     };
}


