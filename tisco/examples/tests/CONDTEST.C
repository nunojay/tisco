
#include "stdio86.inc"


void main()
{
     regs_struc   rin, rout;
     char  i, v[110];

     clrLCD();
     locate(1,1);
     putstr("Cycle test");

     locate(2,2);
     for (i = 0; i < 3; i++)
         putuint(i);
     pause();

     locate(2,2);
     for (i = 0; i <= 3; i++)
         putuint(i);
     pause();
     newline();

     i = 4;
     if (i > 4)       putstr("4>4 ");
     if (i > 3)       putstr("4>3 ");
     if (i > 6)       putstr("4>6 ");
    pause();newline();

     if (i < 4)       putstr("4<4 ");
     if (i < 3)       putstr("4<3 ");
     if (i < 6)       putstr("4<6 ");
    pause();newline();

     if (i >= 4)      putstr("4>=4 ");
     if (i >= 3)      putstr("4>=3 ");
     if (i >= 6)      putstr("4>=6 ");
    pause();newline();

     if (i <= 4)      putstr("4<=4 ");
     if (i <= 3)      putstr("4<=3 ");
     if (i <= 6)      putstr("4<=6 ");
    pause();


     i = 33;
     v[109] = 11;
     v[108] = 22;
     v[107] = 33;
     call_asmprg(" \006EDITOR", &rin, &rout);

     pause();
     clrLCD();
     locate(0,0);
     putstr("Voltei ");
     putuint(i);
     putuint(v[109]);
     putuint(v[108]);
     putuint(v[107]);
}


