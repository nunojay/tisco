
  /*> stdgui.h <*
   *
   * Date: 25/10/1998
   * Last Change: 28/11/1998
   *
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#ifndef __stdgui_h__
#define __stdgui_h__


#ifndef NULL
#define NULL   0x0
#endif


#define RESETMODE    set_wrt_mode(0);
#define SETMODE      set_wrt_mode(1);
#define XORMODE      set_wrt_mode(2);
#define GETMODE      set_wrt_mode(3);


// Drawing mode consts (line and pixel)
#define    dRESET   0
#define    dSET     1
#define    dXOR     2
#define    dGET     3



typedef struct TWIN  {
  uchar   x1;             // Window Total Area
  uchar   y1;
  uchar   x2;
  uchar   y2;
  uchar   maxX, maxY;     // Max relative to window's top left corner
  char    has_border;
  char    *title;         // If NULL, window has no title bar
  int     display_fn;     // Addr of display function (if NULL isn't used)
  TWIN    *next;          // Ptr to next window
}



dll STDGUI {
 uint SDTGUI () export 0;

 void initGUI () export 1;
 void set_wrt_mode (char mode) export 2;
 void clear () export 3;

 void pixel (uchar x, uchar y) export 4;
 uchar getpixel (uchar x, uchar y) export 5;
 void hline (uchar x1, uchar x2, uchar y) export 6;
 void vline (uchar x, uchar y1, uchar y2) export 7;
 void line (uchar x1, uchar y1, uchar x2, uchar y2) export 8;  // has no clip yet
 void rect (uchar x1, uchar y1, uchar x2, uchar y2) export 9;
 void bar (uchar x1, uchar y1, uchar x2, uchar y2) export 10;
 void circle (uchar x, uchar y, uchar ray) export 11;  // not implemented
 void poly () export 12;                               // not implemented
 void fill_area () export 13;                          // not implemented
 void dumm () export 14;                               // not implemented

 void create_window (TWIN *win) export 15;
 void destroy_window (TWIN *win) export 16;  // Still buggy...
 void set_window_title (TWIN *win, char *tit) export 17;
 void set_window_pos (TWIN *win, uchar x, uchar y) export 18;
 void set_window_size (TWIN *win, uchar dx, uchar dy) export 19;
 void show_window (TWIN *win) export 20;
 void clear_window (TWIN *win) export 21;
 void set_top_window (TWIN *win) export 22;
 void change_top_window () export 23;
 void redraw_all () export 24;

 // Still missing some cool text function(s)
}


#endif


