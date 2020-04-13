


#include "crt.h"

#define CHARS_PER_COL    7



void pause ();


void putUChar3 (uchar u)
{
     uchar  aux;
     putChar(u / 100 + '0');
     aux = u % 100;
     putChar(aux / 10 + '0');
     putChar(aux % 10 + '0');
 }



void show_key ()
{
    uchar   ch;
    for (ch = 0; ch != kEXIT; ch = getKey())  {
        gotoxy(1, 1);
        putUChar3(ch);
        putChar(' ');
        putChar(ch);
    }
}


void show_print_table (uchar x, uchar first, uchar lines)
{
   uchar  y, aux, c;

   y = 0;
   for (c = 1; c <= lines; c++)  {
       gotoxy(x, y);
       putUChar3(first);
       putChar(' ');
       putChar(first);
       first++;
       y++;
   }
}


void show_table ()
{
    uchar first, ch;

    first = 0;

    for (ch = 0; ch != kEXIT; )  {
        show_print_table(00, first, CHARS_PER_COL);
        show_print_table(07, first +CHARS_PER_COL, CHARS_PER_COL);
        show_print_table(14, first +CHARS_PER_COL*2, CHARS_PER_COL);

        ch = getKey();
        switch (ch)  {
            case kUP : {
                 if (first != 0)
                    first--;
                 break;  }
            case kDOWN : {
                 if (first != 256-CHARS_PER_COL*3)
                    first++;
                 break;  }
            case kLEFT : {
                 if (first > CHARS_PER_COL-1)
                    first = first - CHARS_PER_COL;
                 break;  }
            case kRIGHT :  {
                 if (first < 256-CHARS_PER_COL*4)
                    first = first + CHARS_PER_COL;
                 break;  }
        }
   }
}


char i;
int  cc, FF;

void main ()
{
    if (4 == i)
        cc = cc - FF;

    initCRT();
    cursorOFF();
    clrScr();
    setViewport(1, 0, 20, 6);
    show_table();
    clrScr();
//    clrViewport();       // NAO TA' FUNCEMINANDO
}

