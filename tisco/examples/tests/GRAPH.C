
#include "stdio86.inc"
#include "math86.inc"
#include "stdgui86.h"




uint random (uint max)
{
     RANDOM();
     uint2OP2(64);
     FPMULT();          // OP1 = random * 64
     return CONVOP1();
}

void t()
{
     clrLCD();locate(0,0);
     putstr("SET");newline();
     for (i = 0; i < 50; i++)
        line(random(126), random(64), random(126), random(64), dSET);
     pause();

     clrLCD();locate(0,0);
     putstr("RESET");newline();
     for (i = 0; i < 50; i++)
        line(random(126), random(64), random(126), random(64), dXOR);
     pause();

     locate(0,0);
     putstr("XOR");newline();
     for (i = 0; i < 50; i++)
        line(random(126), random(64), random(126), random(64), dRESET);
     pause();
}


void testdraw ()
{
     clear();

     putstr("SET");newline();
     set_wrt_mode(dSET);
     line(0, 0, 64, 64);
     pause();
     putstr("RESET");newline();
     set_wrt_mode(dRESET);
     line(2, 0, 66, 64);
     pause();
     putstr("XOR");newline();
     set_wrt_mode(dXOR);
     line(4, 0, 68, 64);
     pause();

     clear();locate(0,0);
     set_wrt_mode(dSET);
     putstr("SET");newline();
     x1 = 0;     y1 = 34;     x2 = 100;     y2 = 45;
     for (i = 0; i < 100; i++)  {
        line(x1, y1, x2, y2);
        x1 = (x1 + y1) & 0x7F;
        x2 = (x1 + y2) & 0x7F;
        y1 = (y1 + y2) & 0x6F;
        y2 = (x2 + y1) & 0x6F;
     }
     pause();
}




void dispfn (TWIN *win) force
{
    char  ym;

    clear_window(win);
    ym = win->maxY >> 1;
    hline(0, win->maxX, ym);
    vline(win->maxX >> 1, 0, win->maxY);
}



uchar  i, x1, x2, y1, y2;
TWIN   w1, w2, w3;
int    vdisp;

void main ()
{
    uchar  y;

//     runindicoff();
     initGUI();
     clear();

     // testdraw();

     for (y1 = 0; y1 <= 50; y1++)
//       for (i = 0; i <= y1; i++)
//           pixel(i, y1);
        hline(61-y1, y1+ 61, y1+1);
     pause();

/*     clear();
     for (y1 = 0; y1 <= 11; y1++)  {
        hline(0, y1, 32);
        pause();
     } /**/

     clear();
     for (x1 = 0; x1 <= 30; x1++)
        vline(x1+1, 30-x1, x1+30);
     pause();

     asm "    ld    hl, _dispfn";
     asm "    ld    (_vdisp), hl";

     w1.title = "First Window";
     set_window_pos (&w1, 20, 20);
     set_window_size (&w1, 60, 30);
     create_window (&w1);
     w1.display_fn = vdisp;  // Must be after create_window
     //w1.has_border = 0;
     w2.title = "2nd Window";
     set_window_pos (&w2, 60, 5);
     set_window_size (&w2, 50, 25);
     create_window (&w2);
     w2.display_fn = vdisp;  // Must be after create_window
     w3.title = NULL;
     set_window_pos (&w3, 5, 10);
     set_window_size (&w3, 25, 25);
     create_window (&w3);
     w3.display_fn = vdisp;

     set_top_window(&w2);
     redraw_all();
     pause();

     for (i = 0; i < 5; i++)  {
         change_top_window();
         pause();
     }

     destroy_window(&w3);
     redraw_all();
     pause();

     clear();
     runindicon();
}


