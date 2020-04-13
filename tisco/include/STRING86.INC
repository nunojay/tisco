
  /*> string86.inc <*
   *
   * Date: 09/11/1998
   * Last Change: 09/11/1998
   *
   * C-style string manipulation.
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */



#ifndef __string86_inc__
#define __string86_inc__



void _strcpy();
void _strlen();
void _strcat();




uint strlen (char str)
{
    getv  str;
    _strlen();
    asm "     ld   l, c";
    asm "     ld   h, b";
}


void strcpy (char *to, char *from)
{
    asm "     ld    d, (ix-3)";    // de = to
    asm "     ld    e, (ix-4)";
    getv  from;
    _strcpy();
}


void strcat (char *to, char *b)
{
    asm "     ld    d, (ix-3)";    // de = to
    asm "     ld    e, (ix-4)";
    getv  b;
    _strcat();
}


uchar strcmp (char *a, char *b)
{
    // yet to be done...
    return 0;
}



#endif

