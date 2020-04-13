


#include "crt.h"

void pause ();


void fillscreen (char ch)
{
   char  x, y;

   gotoxy(0,0);
   for (y = 0; y < crtYRes; y++)
       for (x = 0; x < crtXRes; x++)
           putChar(ch);
}


void show_key ()
{
    char   ch, aux;
    for (ch = 0; ch != kENTER; ch = getKey())  {
        gotoxy(1, 1);
        putChar(ch / 100 + '0');
        aux = ch % 100;
        putChar(aux / 10 + '0');
        putChar(aux % 10 + '0');
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
       putChar(first / 100 + '0');
       aux = first % 100;
       putChar(aux / 10 + '0');
       putChar(aux % 10 + '0');
       putChar(' ');
       putChar(first);
       first++;
       y++;
   }
}


void show_table ()
{
    uchar first, ch;

    clrViewport();
    cursorOFF();
    first = 0;

    for (ch = 0; ch != kENTER; )  {
        show_print_table(0, first, 6);
        show_print_table(7, first+6, 6);
        show_print_table(14, first+6*2, 6);

        ch = getKey();
        switch (ch)  {
            case kUP : {
                 if (first != 0)
                    first--;
                 break;  }
            case kDOWN : {
                 if (first != 256-6*3)
                    first++;
                 break;  }
            case kLEFT : {
                 if (first > 5)
                    first = first - 6;
                 break;  }
            case kRIGHT :  {
                 if (first < 256-6*4)
                    first = first + 6;
                 break;  }
        }
   }
}


void main ()
{
    char  data[16], c;

    initCRT();

    clrScr();
    fillscreen(255);
    setViewport(1, 1, crtXRes-2, crtYRes-2);
    fillscreen(' ');
/*
    show_key();

    gotoxy(0, 0);
    getStr(&data, 15);
    gotoxy(2, 2);
    putStr(&data);/**/

    show_table();

    clrScr();
}

