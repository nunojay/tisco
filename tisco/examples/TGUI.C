
 // This is a more elaborated example with DLLs
 // Compile it with
 //   ..\>tisco TGUI.C
 // It will need STDGUI to be in the calc


#include "stdio86.inc"
#include "stdgui.h"


TWIN   w1, w2, w3;
int    vdisp;



void dispfn (TWIN *win) force
// This is a callback function
{
    clear_window(win);
    hline(0, win->maxX, win->maxY >> 1);
    vline(win->maxX >> 1, 0, win->maxY);
}



void main ()
{
    uchar  i;

     initGUI();
     clear();

     for (i = 0; i <= 50; i++)
        hline(61-i, i+ 61, i+1);
     pause();

     clear();
     for (i = 0; i <= 30; i++)
        vline(i+4, 2+30-i, i+30+2);
     pause();

     // This is the only way to load a variable with the address of a
     // function. However, this doesn't work in DLLs, because TISCO
     // won't generate relocation labels for asm.
     asm "    ld    hl, _dispfn";
     asm "    ld    (_vdisp), hl";

     w1.title = "First Window";
     set_window_pos (&w1, 20, 20);
     set_window_size (&w1, 60, 30);
     create_window (&w1);
     w1.display_fn = vdisp;  // Must be after create_window
//     w1.has_border = 0;

     w2.title = "2nd Window";
     set_window_pos (&w2, 60, 5);
     set_window_size (&w2, 50, 25);
     create_window (&w2);
     w2.display_fn = vdisp;  // Must be after create_window

     w3.title = NULL;       // A titless window
     set_window_pos (&w3, 5, 10);
     set_window_size (&w3, 25, 25);
     create_window (&w3);
     w3.display_fn = vdisp;

     set_top_window(&w2);
     redraw_all();
     pause();

     // Change top window which time ENTER is pressed
     for (i = 0; i < 5; i++)  {
         change_top_window();
         pause();
     }

     destroy_window(&w3);
     redraw_all();
     pause();

     destroy_window(&w2);
     redraw_all();
     pause();

     destroy_window(&w1);
     redraw_all();
     pause();

     clear();
}


