
  // TI-dit
  // TI-86 text editor - NOT FUNCIONAL !!
  //
  // By Viriato 1998
  // This is given without any kind of warranties


#define MAXVAR   2        // Only 2 vars need at the same time
#include "stdv86.inc"
#include "stdio86.inc"
#include "string86.inc"


#define SCREEN_WIDTH     21
#define SCREEN_HEIGHT    7
#define DELTA            512



char   vid_file;
char   *tmpvar_name, vid_tmp;
int    text_size, tmp_size;
char   xPos, yPos;         // Cursor position
bool   text_altered;
int    chPos, ln1Pos;

char   load_var_name[12];



void Quit (char cod)
{
     switch (cod)  {    // Normal exit
        case 0 : {
            delv(tmpvar_name);
            UnlockAlphas();
            SetScroll;
            cursorOn();
            runindicon();
            asm "   set   appTextSave, (iy + appflags)";
            clrScrn();
            homeup();
            break;   }
        case 1 : {
            putstr(tmpvar_name + 2);
            putstr(" exists.");
            break;   }
     }
     exit;
}


char Msg (char cod)
{
     cursorOff();
     clrScrn();
     homeup();
     newline();

     switch (cod)  {
        case 1 : {  putstr(" Var not found"); break;  }
     }

     runindicon();
     pause();
     runindicoff();
     clrScrn();
     cursorOn();
}



char textwidth (char *txt)
{
    char   a;
    a = strlen(txt);
    return a; // << 3 - a;      // a * 7
}


void recreate_tmp (uint size)
{
     delv(tmpvar_name);
     vid_tmp = createv(tmpvar_name, CRTSTRING, size, size, cvOWRT);
     varset(vid_tmp, 2, get_varsize(vid_tmp), 0);    // 0 fill
}



void New ()
// Init for a new text 'file'
{
     // Del temp var and recreate it with size DELTA
     recreate_tmp (DELTA);

     // Init internal variables
     load_var_name[1] = 0;
     load_var_name[11] = 0;
     text_altered = FALSE;
     tmp_size = DELTA;
     xPos = 0;
     yPos = 0;
     chPos = 2;
     ln1Pos = 2;
     text_size = 2;

     // Prepare screen
     clrScrn();
     homeup();
}



bool get_user_var_name ()
{
     char  res;

     clrScrn();
     homeup();
     putstr("VarName?");
     res = get_str(3, 1, &load_var_name[2], 8);
     load_var_name[1] = strlen(&load_var_name[2]);
     return res;
}



void Load (char *name)
// Loads a 'file'
{
     if (get_user_var_name() == FALSE)
        return;

     vid_file = openv(&load_var_name);
     if (vid_file == UNKNOWNVAR)  {
        Msg(1);
        return;
     }

     blkv(vid_file, &text_size, 2, VREAD);   // Read size
     set_varsize(vid_file, text_size);

     recreate_tmp (text_size + DELTA);

     copyv(vid_tmp, 2, vid_file, 2, text_size);  // text -> tmp
     text_size = text_size + 2;

     closev(vid_file);
}



void Save (char *name)
{
     // Ask var's name if we don't know it yet
     if (load_var_name[1] == 0)
        if (get_user_var_name() == FALSE)
           return;

     delv(&load_var_name);   // We should check if it exists

     vid_file = createv(&load_var_name, CRTSTRING,
                        text_size-2, text_size-2, cvOWRT);
     copyv(vid_file, 2, vid_tmp, 2, text_size - 2);  // tmp -> text
}



//***********************************************************************

void clear_text_shadow ()
{
     asm {
      ld     hl, _textShadow
      ld     bc, (21*8 *256) + 32
cts_loop:
      ld     (hl), c
      inc    hl
      djnz   cts_loop
     };
}


void putc_ts (char ch)
{
     int  ofs;
     ofs = curRow * SCREEN_WIDTH + curCol;
     getv  ofs;
     asm "     ld    de, _textShadow";
     asm "     add   hl, de";
     getv  ch;
     asm "     ld    (hl), a";
}


bool incrRow ()
{
     if (curRow == SCREEN_HEIGHT-1)
        return TRUE;    // End of the screen
     curRow++;
     curCol = 0;
     return FALSE;
}


bool incrCol ()
{
     if (curCol == SCREEN_WIDTH-1)
        return incrRow();
     curCol++;
     return FALSE;
}


void print_text ()
// Prints a text page into the shadow buffer
{
     char   ch;

     clear_text_shadow ();
     seekv(vid_tmp, ln1Pos);
     curCol = 0;
     curRow = 0;

     for (;;)  {
         if (chPos == get_varpos(vid_tmp))  {
            xPos = curCol;
            yPos = curRow;
         }

         ch = getcv(vid_tmp);

         if (ch == NULL)
            break;

         if (ch == kENTER)  {
            if (incrRow())
               break;
         }
         else  {
            putc_ts(ch);
            if (incrCol())
               break;
         }
     }
}



void open_space (int pos, int size)
{
     copyv(vid_tmp, pos+size, vid_tmp, pos, text_size-pos);
     text_size = text_size + size;
}


void del_block (int pos, char size)
{
     copyv(vid_tmp, pos, vid_tmp, pos+size, text_size-pos);
     text_size = text_size - size;
}



int advance (int vl)
{
     uint   end;

     end = vl + SCREEN_WIDTH - 1;
     seekv(vid_tmp, vl);
     for (; vl <= end; vl++)
         switch (getcv(vid_tmp))  {
            case NULL :
                 { vl--; return vl; }
            case kENTER :
                 { vl++; return vl; }
         }
     return vl;
}

void advance_ln1 ()
{
     ln1Pos = advance(ln1Pos);
}


void backen_ln1 ()
{
     uint   end, lsize;

     if (ln1Pos <= SCREEN_WIDTH)  {
        ln1Pos = 2;
        return;
     }
     end = ln1Pos - SCREEN_WIDTH;
     seekv(vid_tmp, ln1Pos);
     for (; ln1Pos > end; ln1Pos--)
         if (getcv(vid_tmp) == kENTER)
            break;
     if (ln1Pos == end)
        return;

     // The more complicated case...
     // Find the size of the last line.
     end = ln1Pos;
     for (ln1Pos--; ln1Pos > 2; ln1Pos--)
         if (getcv(vid_tmp) == kENTER)
            break;

     lsize = end - ln1Pos;
     if (lsize < SCREEN_WIDTH)  {  // Last line is short and fits
        ln1Pos++;                  // in a screen line
        return;
     }

     ln1Pos = ln1Pos + lsize % SCREEN_WIDTH;
}



/**********************************************************************/

char  ch;



void insert_char ()
{
     open_space(chPos, 1);
     seekv(vid_tmp, chPos);
     putcv(vid_tmp, ch);
     chPos++;
}


void do_kLEFT ()
{
     if (chPos <= 2)
        return;
     chPos--;
     if (yPos == 0)
        if (xPos == 0)
           backen_ln1 ();
}


void do_kRIGHT ()
{
     if (chPos == text_size)
        return;
     chPos++;
     if (yPos == SCREEN_HEIGHT-1)
        if (xPos == SCREEN_WIDTH-1)
           advance_ln1 ();
}


void do_kDEL ()
{
     if (chPos <= 2)
        return;
     del_block(chPos - 1, 1);
     do_kLEFT();
}


void do_kENTER ()
{
     insert_char();
     if (yPos == SCREEN_HEIGHT-1)
        advance_ln1();
}


void do_kDOWN ()
{
     chPos = advance(chPos);
     if (yPos == SCREEN_HEIGHT-1)
        advance_ln1();
}



void tblit ()
{
         cursorOff();

         print_text();
         cpyText();

         curRow = 7;
         curCol = 0;
         putuint(ln1Pos);
         putuint(chPos);

         curRow = yPos;
         curCol = xPos;
         SetLowerAlphaLock();
         cursorOn();
}





void main ()
{
     runindicoff();
     SetLowerAlphaLock();
     SetNoScroll;
     asm "   res   appTextSave, (iy + appflags)";
     winTop = 0;
     winBtm = SCREEN_HEIGHT;
     tmpvar_name = " \010zTIdiTmp";

     if (findv(tmpvar_name))
        Quit(1);

     New();
     cursorOn();


     while (TRUE)  {
         ch = readkey();

         switch (ch)  {
           case kEXIT : Quit(0);      // Doesn't return

           case kDEL :
                {  do_kDEL(); break;  }
           case kENTER :
                {  do_kENTER(); break;  }
           case kUP :
             {  backen_ln1(); break;  }
           case kDOWN :
                {  do_kDOWN(); break;  }
           case kLEFT :
                {  do_kLEFT(); break;  }
           case kRIGHT :
                {  do_kRIGHT(); break;  }
           case kF3 :
                {  Load(); break;  }
           case kF2 :
                {  Save(); break;  }
           default :  {
                insert_char();
                chPos--;       // [compensate chPos++ in do_kRIGHT()]
                do_kRIGHT();  }
         }

         tblit();
     }
}


