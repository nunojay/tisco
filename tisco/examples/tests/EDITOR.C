
  // Simple text editor
  // Not done!
  // I was planning to release a text editor (I haven't seen
  // any usefull one yet) as the first release compiled with TISCO.


#include "stdv86.inc"
#include "stdio86.inc"
#include "string86.inc"



char   vid, data[5], i;
char   xPos, yPos;



void init ()
{
     clrScrn();
     vlocate(0, 0);
     locate(2,3);



     vid = createv(" \007MANHOSO", CRTSTRING, 128, 128, cvEXCL);

     if (vid != VAREXISTS)  {
        vputstr("MANHOSO created");
        seekv(vid, 2);
        blkv(vid, "OLA ola", 8, VWRITE);
        get_vartype(vid);
     }
     else  {
        vputstr("MANHOSO already exists");
        vid = openv(" \007MANHOSO");

        // ... here we should read the var's size from the var herself
        set_varsize(vid, 128);
        seekv(vid, 2);
        blkv(vid, &data, 8, VREAD);
        putstr(&data);
        get_vartype(vid);
     }
}



void main ()
{
   char  arr[16];
   char  ch;

     clrScrn();

//     get_str(1, 1, &arr, 15);
     cursorOn();
     for (ch = readkey(); ch != 6; ch = readkey())  {
//         locate(1,1);
         getv ch;
         putc();
     }
     cursorOff();

     init();
     pause();
     putuint(vid);
}


