
  /*> stdgui.c <*
   *
   * Date: 25/10/1998
   * Last Change: 01/12/1998
   *
   * Graphics drawing and GUI routines.
   * More to show TISCO capabilitys than to be usefull...
   * Compile with
   *     ..\>tisco STDGUI.C -d -jo
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#include "stdgui.h"


#define setmode    write_mode = 1
#define resmode    write_mode = 0

#define LCD_ADDR         0xFC00
#define BYTES_PER_LINE   16


char  write_mode;    // Write mode for drawing routines

char  PixOfsTab__[8];
char  PixOfsTab1__[8];
char  PixOfsTab2__[8];


// ViewPort limits
uchar  xMin, xMax, yMin, yMax;


/**************************************************************************/
/** External declarations                                                **/

extern uchar penRow;
extern uchar penCol;

void clrLCD ();
void clrScrn ();
void vputs ();




/**************************************************************************/
/** DLL Version Function                                                 **/

uint SDTGUI () export 0
{
    return 0*256+0;         // Version 0.0
}


/**************************************************************************/
/** DLL Internal Functions                                               **/

uchar  *addr;   // Used to hold the actual byte in LCD mem. As it's a
                // 16bit var, it's smaller and faster to have it as global.


void set_viewport_all ()
{
    xMin = 0;
    yMin = 0;
    xMax = 127;
    yMax = 63;
}


void calc_addr (uchar x, uchar y)
// Puts the LCD mem addr of point (x,y) into var addr
{
    addr = 63 - y;   // Y axis inversion and conversion to 16bits
    addr = LCD_ADDR + addr * BYTES_PER_LINE;
    addr = addr + x >> 3;
}


void put_mask_pixel (uchar mask)
// Puts a pixel mask according to actual write mode
{
    uchar  aux;

    aux = *addr;
    if (write_mode == dSET)
       aux = mask | aux;
    else if (write_mode == dRESET)
       aux = (mask ^ 0xFF) & aux;
    else
       aux = mask ^ aux;
    *addr = aux;
}



/**************************************************************************/
/** Basic functions                                                      **/

void initGUI () export 1
{
    asm "    res   textwrite, (iy + new_grf_flgs)";
    asm "    set   appTextSave, (iy + appflags)";
    setmode;

    set_viewport_all ();

    PixOfsTab__[0] = 0x80;   // 10000000
    PixOfsTab__[1] = 0x40;   // 01000000
    PixOfsTab__[2] = 0x20;   // 00100000
    PixOfsTab__[3] = 0x10;   // 00010000
    PixOfsTab__[4] = 0x08;   // 00001000
    PixOfsTab__[5] = 0x04;   // 00000100
    PixOfsTab__[6] = 0x02;   // 00000010
    PixOfsTab__[7] = 0x01;   // 00000001

//    PixOfsTab1__[0] = 0x00;
    PixOfsTab1__[1] = 0x7F;  // 01111111
    PixOfsTab1__[2] = 0x3F;  // 00111111
    PixOfsTab1__[3] = 0x1F;  // 00011111
    PixOfsTab1__[4] = 0x0F;  // 00001111
    PixOfsTab1__[5] = 0x07;  // 00000111
    PixOfsTab1__[6] = 0x03;  // 00000011
    PixOfsTab1__[7] = 0x01;  // 00000001

    PixOfsTab2__[0] = 0x80;  // 10000000
    PixOfsTab2__[1] = 0xC0;  // 11000000
    PixOfsTab2__[2] = 0xE0;  // 11100000
    PixOfsTab2__[3] = 0xF0;  // 11110000
    PixOfsTab2__[4] = 0xF8;  // 11111000
    PixOfsTab2__[5] = 0xFC;  // 11111100
    PixOfsTab2__[6] = 0xFE;  // 11111110
    PixOfsTab2__[7] = 0xFF;  // 11111111
}


void set_wrt_mode (char mode) export 2
{
    write_mode = mode;
}


void clear () export 3
// Clears LCD and sets SET mode
{
    clrLCD();
    setmode;
}



void pixel (uchar x, uchar y) export 4
// Puts a pixel
{
    x = x + xMin;
    y = y + yMin;
    if (x < xMin)  return;
    if (x > xMax)  return;
    if (y < yMin)  return;
    if (y > yMax)  return;
    calc_addr(x, y);
    put_mask_pixel(PixOfsTab__[x & 0x07]);
}


uchar getpixel (uchar x, uchar y) export 5
// Gets a pixel. Returns 0 if pixel not set, != 0 otherwise.
{
    x = x + xMin;
    y = y + yMin;
    calc_addr(x, y);
    return *addr & (PixOfsTab__[x & 0x07] ^ 0xFF);
}


void hline (uchar x1, uchar x2, uchar y) export 6
{
    uchar  mask, x, xa, xb, xu;

    // Viewport and cliping
    x1 = x1 + xMin;
    x2 = x2 + xMin;
    y = y + yMin;
    if (y > yMax)  return;
    if (y < yMin)  return;
    if (x1 > x2)  {
       x = x1; x1 = x2; x2 = x;
    }
    if (x1 > xMax)  return;
    if (x2 < xMin)  return;
    if (x1 < xMin)  x1 = xMin;
    if (x2 > xMax)  x2 = xMax;

    // draw
    calc_addr(x1, y);
    xa = x1 & 0x07;
    xb = x2 & 0x07;
    x = (x1 + 7) & 0xF8;    // next 8 bit boundary >= x1
    xu = x2 & 0xF8;         // previous 8bit boundary <= x2

    if (x2 - x1 > 8)  {
       if (xa)  {  // set 1st byte, if there is one
          put_mask_pixel(PixOfsTab1__[xa]);
          addr++;
       }

       if (xu + 8 >= x)
         for (; x < xu; x = x + 8)  {
             put_mask_pixel(0xFF);
             addr++;
         }

       put_mask_pixel(PixOfsTab2__[xb]);
    }
    else  {
     // line(x1, y, x2, y);
        mask = PixOfsTab__[x1 & 0x07];
        for (x = x1; x <= x2; x++)  {
            put_mask_pixel(mask);
            mask = mask >> 1;
            ifz  {
               mask = 0x80;
               addr++;
            }
        }
    }
}


void vline (uchar x, uchar y1, uchar y2) export 7
{
    uchar  mask, aux;

    // Viewport and cliping
    x = x + xMin;
    y1 = y1 + yMin;
    y2 = y2 + yMin;
    if (x > xMax)  return;
    if (x < xMin)  return;
    if (y1 > y2)  {
       aux = y1; y1 = y2; y2 = aux;
    }
    if (y1 > yMax)  return;
    if (y2 < yMin)  return;
    if (y1 < yMin)  y1 = yMin;
    if (y2 > yMax)  y2 = yMax;

    // Draw
    calc_addr(x, y1);
    mask = PixOfsTab__[x & 0x07];

    for (; y1 <= y2; y1++)  {
        put_mask_pixel(mask);
        addr = addr - BYTES_PER_LINE;
    }
}


void line (uchar x1, uchar y1, uchar x2, uchar y2) export 8
// Draws a line (no clip)
{
    x1 = x1 + xMin;
    x2 = x2 + xMin;
    y1 = y1 + yMin;
    y2 = y2 + yMin;

    getv write_mode;     // mode: 0-dRESET; 1-dSET; 2-dXOR
    asm {
    ld     h, a
    ld     b, (ix - 4)       ; x1
    ld     c, (ix - 3)       ; y1
    ld     d, (ix - 2)       ; x2
    ld     e, (ix - 1)       ; y2
    push   ix
    call   _ILine
    pop    ix
    };
}


void rect (uchar x1, uchar y1, uchar x2, uchar y2) export 9
// Draws a rectangle
{
    hline(x1, x2, y1);
    hline(x1, x2, y2);
    vline(x1, y1, y2);
    vline(x2, y1, y2);
}


void bar (uchar x1, uchar y1, uchar x2, uchar y2) export 10
// Draws a bar (no clip)
{
    char  aux;

    if (y1 > y2)  {
       aux = y1; y1 = y2; y2 = aux;
    }
    for (; y1 <= y2; y1++)
        hline(x1, x2, y1);
}


void circle (uchar x, uchar y, uchar ray) export 11  {}
void poly () export 12  {}
void fill_area () export 13  {}

void dumm () export 14  {}


/**************************************************************************/
/** More advanced functions                                              **/

TWIN  *WinLst;    // List of windows (auto-init NULL)
TWIN  *TopWin;    // The window on top of all others (auto-init NULL)

TWIN  *awin;      // Auxiliar. Global var gens less code than local.
TWIN  *awin2;     // dido. use by functions wich call functions that
                  // use awin.

/*
   Windows can have
       * Title bar
       * Border

   Window Related Functions
       * create_window
       * destroy_window
       * set_window_title
       * set_window_pos
       * set_window_size
       * show_window (or bring window to top)
       * clear_window (clear the client's area)
       * set_top_window
       * change_top_window (another window goes 'on top')
       * redraw_all (full redraw)
*/


void call_back (TWIN *win)
{
    if (win->display_fn)  {
       getv win->display_fn;
       asm "     dec   ix";    // Pass win as parameter
       asm "     dec   ix";
       asm "     jp    (hl)";
    }
}


void set_viewport (TWIN *win)
{
    set_viewport_all();
    awin = win;
    if (awin)  {
       xMin = awin->x1;
       yMin = awin->y1;
       xMax = awin->x2;
       yMax = awin->y2;
    }
}



void create_window (TWIN *win) export 15
{
    // Insert window in the list
    win->next = WinLst;
    WinLst = win;
    set_top_window(WinLst);
    WinLst->display_fn = NULL;
    WinLst->has_border = 1;
}


void destroy_window (TWIN *win) export 16
// STILL has a bug
{
    awin2 = win;
    // Del window from list
    if (WinLst == awin2)
       WinLst = WinLst->next;
    else  {
       // Search window and then
       for (awin = WinLst; awin->next; awin = awin->next)
           if (awin->next == awin2)
              break;
       // del it from the window list
       if (awin->next)
          awin = awin->next->next;
    }
    set_top_window(WinLst);
}


void set_window_title (TWIN *win, char *tit) export 17
// Sets a window title.
{
    awin = win;
    awin->title = tit;
    if (tit == NULL)  return;

    set_viewport_all();
    resmode;
    hline(awin->x1 -2, awin->x2 +2, awin->y2 +8);
    vline(awin->x1 -2, awin->y2 -8, awin->y2 +8);
    vline(awin->x2 +2, awin->y2 -8, awin->y2 +8);
    setmode;
    // Draw Title Bar
    bar(awin->x1-1, awin->y2+1, awin->x2+1, awin->y2+7);

    penRow = 56 - awin->y2;
    penCol = awin->x1 + 1;
    asm "    set   textInverse, (iy + textflags)";
    getv  tit;
    vputs();
    asm "    res   textInverse, (iy + textflags)";
    set_viewport(TopWin);
}


void set_window_pos (TWIN *win, uchar x, uchar y) export 18
{
    awin = win;
    awin->x1 = x;
    awin->y1 = y;
    set_window_size (awin, awin->maxX, awin->maxY);
/*    awin->x2 = x + awin->maxX;
    awin->y2 = y + awin->maxY; /**/
}


void set_window_size (TWIN *win, uchar dx, uchar dy) export 19
{
    awin = win;
    awin->maxX = dx;
    awin->maxY = dy;
    awin->x2 = awin->x1 + dx;
    awin->y2 = awin->y1 + dy;  /**/
}


void show_window (TWIN *win) export 20
// Draws a square with a title...
{
    uchar x1, y1, x2, y2;

    if (win == NULL)  return;

    awin2 = win;
    x1 = awin2->x1;
    y1 = awin2->y1;
    x2 = awin2->x2;
    y2 = awin2->y2;

    set_viewport_all();
    // Clear a 1 pixel wide border outside the window
    resmode;

    // Draw border
    if (awin2->has_border)  {
       rect (x1 - 2, y1 - 2, x2 + 2, y2 + 2);
       setmode;
       rect (x1 - 1, y1 - 1, x2 + 1, y2 + 1);
    }
    else
       rect (x1 - 1, y1 - 1, x2 + 1, y2 + 1);   // in RES write mode

    set_window_title (awin2, awin2->title);

    set_viewport(awin2);
    call_back (awin2);   // Invoque display function
    set_viewport(TopWin);
}


void clear_window (TWIN *win) export 21
// Clears the inside of a window and sets SET mode
{
    awin = win;
    set_viewport_all ();
    resmode;
    bar(awin->x1, awin->y1, awin->x2, awin->y2);
    setmode;
    set_viewport(win);
}


void set_top_window (TWIN *win) export 22
{
   TopWin = win;
   set_viewport(TopWin);
}


void change_top_window () export 23
{
    if (TopWin)  {
       TopWin = TopWin->next;
       if (TopWin == NULL)
          TopWin = WinLst;
       show_window(TopWin);
    }
}


void redraw_all () export 24
{
    clear();
    for (awin2 = WinLst; awin2; awin2 = awin2->next)
        if (awin2 != TopWin)
           show_window(awin2);

    show_window(TopWin);
}





