
 /** Gerado por plexer v1.0 */

#ifndef plTICLEX__h
#define plTICLEX__h

#include <stdio.h>

/* Valores para os tokens */
enum ttoken  { 
        INTEIRO = 256, STRING = 257, NREAL = 258, CAR = 259, 
        ID = 260, EOT = 261, PPINCLUDE = 262, PPDEFINE = 263, 
        PPUNDEF = 264, PPIFDEF = 265, PPIFNDEF = 266, PPENDIF = 267, 
        PPERROR = 268, GETE = 269, GETV = 270, VOID = 271, 
        CHAR = 272, UCHAR = 273, INT = 274, UINT = 275, 
        RETURN = 276, IF = 277, IFC = 278, IFNC = 279, 
        IFZ = 280, IFNZ = 281, FOR = 282, ELSE = 283, 
        WHILE = 284, TYPEDEF = 285, STRUCT = 286, SWITCH = 287, 
        BREAK = 288, CASE = 289, DEFAULT = 290, ASM = 291, 
        EXTERN = 292, FORCE = 293, EXPORT = 294, DLL = 295, 
        IGUAL = 296, DIFERENTE = 297, MAIOR = 298, MENOR = 299, 
        MAIOROUIGUAL = 300, MENOROUIGUAL = 301, MAISMAIS = 302, MENOSMENOS = 303, 
        SETA = 304, SHL = 305, SHR = 306
  };

typedef enum ttoken ttoken;


#define NOVA_LINHA    123456         // token especial...    (special token (NEWLINE))



typedef struct {
   ttoken  token;         /* O token */
   union {                /* O valor do token */
      char  c;
      int   i;            /* Constantes numericas inteiras */
      float f;            /* floats... */
      char  *lexema;      /* Identificadores ou strings */
   } u;
 } TTOKEN;


typedef struct TLSTTOKENS {        /* Lista de tokens */
    TTOKEN  *t;
    struct TLSTTOKENS  *ante;
    struct TLSTTOKENS  *prox;
 } TLSTTOKENS;


typedef struct TFINFO {
   char   nome[512];
   int    nlin;
   int    ncol;
   FILE   *fin;
 } TFINFO;



// Da' info sobre a pos do token actual
extern TFINFO  *filep;


//typedef int (*TCBPREP)(TTOKEN *tok);

int lex (char*, char*);
//void pushf (char *novo);
TTOKEN *gettoken ();
int lookstr (char*, char*, int);

void printt (TTOKEN*);



#endif






