
#define PUSHW      asm "    .db \"eieo\"";
#define POPB       asm "    pop    af";

#define LD_DE_HL   asm "    ld     e, l";  \
                   asm "    ld     d, h";

#define TEST       c = 1;         \
                   c = 1;         \
                   while (c) c++;
#define STRING     0
#define REAL


  /** Test file **/

int addr; char gg;

void cc () force
{
   char a, b, f, g, h, k, k1, e, d, s, c;

   a = c * ((c - b)) / d;  // ccb-*d/
   a = c * ((c / b)) / d;  // ccb/*d/
   a = c * (c / b) / d;
    // ccb/*d/
   a = c * c / b / d;
    // cc*b/d/
   a = b - (c + f + d) * (c / (b-1)) / d - (s * d);
    // bcf+cb1-/*d/-sd*-
   a = a - b + c;
   a = a - b * c + d * (e + f);

   k = a << b;
   k = a << 5;
   k = a >> 1;
   k = 1 << 1;
   k = 1 << a;
/*    k = 2 + a + 3;
    k = (2 + a) + 3;
    k = 2 + (a + 3);
    k = 2 * a + 3;
    k = 2 + a * 3;
    k = 2 + a * 3 * h;
    k = a * 3 * h + 2;

    k = - 2 + a - b - 3 + g - 3 - h + a;
    k = 3 + (g - (3 - (h * (a + (b -2)))));
    k = k;
    k = k1 / (2 / a) / b - 3 + -(g - 3) - h + a * (b - 2);
    k = k;
    k = k1 - 2 + a - b - 3 + (g - 3) - h + a + b - -2; */
}


typedef char  *row[2*2];
typedef row   matrix[2*2];

typedef struct TT {
    char   dum;
    char   c;
    int    i;
    char   *pc;
    int    *pi;
    TT     *prox;
}

typedef struct TT1 {
    char   dum;
    char   c;
    int    i;
    char   *pc;
    int    *pi;
    TT     s;
    TT     *ps;
    TT1    *prox;
}



char Void (char arg8, int arg16, char *ptr8, int *ptr16)
{ return 0; }

char Void1A (int *ptr16)
{ return 0; }


char gc;
int  gi;

TT1  vecTT1[10], *pTT1;
char index;


void test_locals (char arg8, int arg16, char *ptr8, int *ptr16)
{
    char   a1, b1, c1, c;
    int    a2, b2, c2, i, i1;
    char   *ap1, *bp1;
    int    *ap2, *bp2;
    TT     t, *pt;
    TT1    t1, *pt1;



/*    i = c1 * (2 + a1) + c;
    i = c1 * ((2 + a1) * c) + c;

    c = c1 * 2 - 3 + c;
    c = c1 * 2 + 3 + c;
    i = i1 * 2 - 3 + i;
    i = i1 * 2 + 3 + i;*/

    pTT1 = &vecTT1[index];
//    pTT1 = vecTT1[index];
    pTT1 = &vecTT1[index +1];
//    pTT1 = vecTT1[index +index+5];

    PUSHW;
    ap1 = "ola\"\"";
    LD_DE_HL;

//#undef REAL
#ifndef KK
#ifdef REAL
    TEST;

    c = STRING;

    for (gi = 0; gi; gi++) c = 0;;
    for (gc = 0; gc; gc--) c = 0;;
    c = i + c - c;

    for (i = 0; i; i++) c = 0;;
    for (c --; c; c++) i = 0;;
    i++;
    c--;
    ifz
    i = 32;
    ifnc
    c = 4;
    else
    c1 = 8;
    i1 = 16;
  // Test atribs
    c = c;            // 8 = 8
    c = i;            // 8 = 16
    i = i;            // 16 = 16
    i = c;            // 16 = 8

  // Test complex atribs
    c = t.c;
    c = pt1->c;       // 8 = seta8
    i = pt1->c;       // 16 = seta8
    c = pt1->i;       // 8 = seta16
    i = pt1->i;       // 16 = seta16
    c = t1.s.c;
    c = t1.s.pc;
    c = pt1->ps->c;

  // Test simple expressions
    c = c + c1;       // 8 = 8 + 8
    c = c1 + i;       // 8 = 8 + 16
    c = i + c1;       // 8 = 16 + 8
    i = i + i1;       // 16 = 16 + 16
    i = i + c1;       // 16 = 8 + 16
    i = c + i1;       // 16 = 16 + 8
    c = 1 + c;        // 8 = k + 8
    c = c + 1;        // 8 = 8 + k
    c = 1 + i;        // 8 = k + 16
    c = i + 1;        // 8 = 16 + k
    i = 1 + i;        // 16 = k + 16
    i = i + 1;        // 16 = 16 + k
    i = 1 + c;        // 16 = k + 8
    i = c + 1;        // 16 = 8 + k
#endif

  // Test more complex expressions
    c = c + c1 + c;   // 8 = 8 + 8 + 8
    c = c / c1 / c;   // 8 = 8 / 8 / 8
    c = c / c1 * c;   // 8 = 8 / 8 * 8
    c = c - c1 - c;   // 8 = 8 - 8 - 8
    c = c - c1 - 1;   // 8 = 8 - 8 - k
    c = c - c1 / c;   // 8 = 8 - 8 / 8
    i = i - i1 - i;   // 16 = 16 - 16 - 16
    i = i + i1 + i;   // 16 = 16 + 16 + 16
    i = i - i1 - 1;   // 16 = 16 - 16 - k
    i = i - i1 / i;   // 16 = 16 - 16 / 16
    c = c - i1 - c;   // 8 = 8 - 16 - 8
    c = c - i1 - 1;   // 8 = 8 - 16 - k
    c = c - i1 / c;   // 8 = 8 - 16 / 8
    i = i - c - i;    // 16 = 16 - 8 - 16
    i = i - c - 1;    // 16 = 16 - 8 - k
    i = i - c / i;    // 16 = 16 - 8 / 16
    i = i << i1 + i | i - 1;     // 16 = 16 << 16 + 16 | 16 - k
    i = c >> i - i1 & c + 1;     // 16 = 8 >> 16 - 16 & 8 + k

  // Test conditions
    if (c);           // 8
    if (i);           // 16
    if (2);           // k
    if (c == 1);      // 8 == k
    if (1 == c);      // k == 8
    if (c == i);      // 8 == 16
    if (i == c);      // 16 == 8
    if (i == 2);      // 16 == k
    if (2 == i);      // k == 16
    if (c == c1);     // 8 == 8
    if (i == i1);     // 16 == 16
    if (2 == 2);      // k == k
    if (c)
       c = 0;
    else
       c = 1;
    if (i)
       i = 0;
    else
       i = 1;
    if (c == 2)  {
       c = 0;
       i = 1;
     }
    else  {
       c = 1;
       i = 0;
    }
#endif

    // Test calls
    test_locals(0,0,0,0);
    Void(0,0,0,0);
    Void(c,c,c,c);
    Void(i,i,i,i);
    Void(gi,i,gi,i);
    Void(c,i,i,i);          // No casts
    Void(2, "rdfr", "-", "k");
    Void(c - 1, i + 1, i - 1, i * 1);
    Void(Void(0,1,2,3), -1,5,6);

    // Test asm
    asm {          // Can't have nothing more next to '{'
      ld    a, 2          ; Here the comments are TASM-like;
      xor   b             ; the text is simply copied to output
    };

    for (c = 0; c < 10; c = c + 1);
    for (i = 0; i < 10; i = i + 1);

    for (c = 0; c < 10; c = c + 1)  {
        if (c == -1)
           break;
        for (c = 0; c < 10; c = c + 1)  {
            return;
            break;
            return;
        }
        c = c + 1;
    }
    return 0;
}


char   a1, b1, c1, c;
int    a2, b2, c2, i, i1;
char   *ap1, *bp1;
int    *ap2, *bp2;
TT     t, *pt;
TT1    t1, *pt1;



int test_globals (char arg8, int arg16, char *ptr8, int *ptr16)
{
    c = i + c - c;
    i = c;

    for (i = 0; i; i++) c = 0;;
    for (c --; c; c++) i = 0;;
    i++;
    c--;
    ifz
    i = 32;
    ifnc
    c = 4;
    else
    c1 = 8;
    i1 = 16;
  // Test atribs
    c = c;            // 8 = 8
    c = i;            // 8 = 16
    i = i;            // 16 = 16
    i = c;            // 16 = 8

  // Test complex atribs
    c = t.c;
    c = pt1->c;
    c = t1.s.c;
    c = pt1->ps->c;

  // Test simple expressions
    c = c + c1;       // 8 = 8 + 8
    c = c1 + i;       // 8 = 8 + 16
    c = i + c1;       // 8 = 16 + 8
    i = i + i1;       // 16 = 16 + 16
    i = i + c1;       // 16 = 8 + 16
    i = c + i1;       // 16 = 16 + 8
    c = 1 + c;        // 8 = k + 8
    c = c + 1;        // 8 = 8 + k
    c = 1 + i;        // 8 = k + 16
    c = i + 1;        // 8 = 16 + k
    i = 1 + i;        // 16 = k + 16
    i = i + 1;        // 16 = 16 + k
    i = 1 + c;        // 16 = k + 8
    i = c + 1;        // 16 = 8 + k

  // Test more complex expressions
    c = c + c1 + c;   // 8 = 8 + 8 + 8
    c = c / c1 / c;   // 8 = 8 / 8 / 8
    c = c / c1 * c;   // 8 = 8 / 8 * 8
    c = c - c1 - c;   // 8 = 8 - 8 - 8
    c = c - c1 - 1;   // 8 = 8 - 8 - k
    c = c - c1 / c;   // 8 = 8 - 8 / 8
    i = i - i1 - i;   // 16 = 16 - 16 - 16
    i = i + i1 + i;   // 16 = 16 + 16 + 16
    i = i - i1 - 1;   // 16 = 16 - 16 - k
    i = i - i1 / i;   // 16 = 16 - 16 / 16
    c = c - i1 - c;   // 8 = 8 - 16 - 8
    c = c - i1 - 1;   // 8 = 8 - 16 - k
    c = c - i1 / c;   // 8 = 8 - 16 / 8
    i = i - c - i;    // 16 = 16 - 8 - 16
    i = i - c - 1;    // 16 = 16 - 8 - k
    i = i - c / i;    // 16 = 16 - 8 / 16
    i = i << i1 + i | i - 1;     // 16 = 16 << 16 + 16 | 16 - k
    i = c >> i - i1 & c + 1;     // 16 = 8 >> 16 - 16 & 8 + k

  // Test conditions
    if (c);           // 8
    if (i);           // 16
    if (2);           // k
    if (c == 1);      // 8 == k
    if (1 == c);      // k == 8
    if (c == i);      // 8 == 16
    if (i == c);      // 16 == 8
    if (i == 2);      // 16 == k
    if (2 == i);      // k == 16
    if (c == c1);     // 8 == 8
    if (i == i1);     // 16 == 16
    if (2 == 2);      // k == k

    // Test calls
    test_locals(0,0,0,0);
    Void(0,0,0,0);
    Void(c,c,c,c);
    Void(i,i,i,i);
    Void(c,i,i,i);          // No casts
    Void(-2,"rdfr", "-", "k");
    Void(c - 1, i + 1, i - 1, i * 1);
    Void(Void(0,1,2,3), 4,5,6);

    // Test asm
    asm {          // Can't have nothing more next to '{'
      ld    a, 2          ; Here the comments are TASM-like;
      xor   b             ; the text is simply copied to output
    };

    for (c = 0; c < 10; c = c + 1);
    for (i = 0; i < 10; i = i + 1);

    return c;
    return i;
}



int main ()
{
    test_locals();
    test_globals();
    return 0;
}



