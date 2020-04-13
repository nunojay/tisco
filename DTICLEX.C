
 /** Gerado por pLexer v1.0 */
 /** Rotina para imprimir os tokens (DEBUG) */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "TICLEX.h"



void printt (TTOKEN *t)
{
     if (t->token < 256)
         printf("%c ", t->token);
     else
      switch (t->token)  {
        case INTEIRO : printf("INTEIRO "); break;
        case STRING : printf("STRING "); break;
        case NREAL : printf("NREAL "); break;
        case CAR : printf("CAR "); break;
        case ID : printf("ID "); break;
        case EOT : printf("EOT "); break;
        case PPINCLUDE : printf("include "); break;
        case PPDEFINE : printf("define "); break;
        case PPUNDEF : printf("undef "); break;
        case PPIFDEF : printf("ifdef "); break;
        case PPIFNDEF : printf("ifndef "); break;
        case PPENDIF : printf("endif "); break;
        case PPERROR : printf("error "); break;
        case GETE : printf("gete "); break;
        case GETV : printf("getv "); break;
        case VOID : printf("void "); break;
        case CHAR : printf("char "); break;
        case UCHAR : printf("uchar "); break;
        case INT : printf("int "); break;
        case UINT : printf("uint "); break;
        case RETURN : printf("return "); break;
        case IF : printf("if "); break;
        case IFC : printf("ifc "); break;
        case IFNC : printf("ifnc "); break;
        case IFZ : printf("ifz "); break;
        case IFNZ : printf("ifnz "); break;
        case FOR : printf("for "); break;
        case ELSE : printf("else "); break;
        case WHILE : printf("while "); break;
        case TYPEDEF : printf("typedef "); break;
        case STRUCT : printf("struct "); break;
        case SWITCH : printf("switch "); break;
        case BREAK : printf("break "); break;
        case CASE : printf("case "); break;
        case DEFAULT : printf("default "); break;
        case ASM : printf("asm "); break;
        case EXTERN : printf("extern "); break;
        case FORCE : printf("force "); break;
        case EXPORT : printf("export "); break;
        case DLL : printf("dll "); break;
        case IGUAL : printf("== "); break;
        case DIFERENTE : printf("!= "); break;
        case MAIOR : printf("> "); break;
        case MENOR : printf("< "); break;
        case MAIOROUIGUAL : printf(">= "); break;
        case MENOROUIGUAL : printf("<= "); break;
        case MAISMAIS : printf("++ "); break;
        case MENOSMENOS : printf("-- "); break;
        case SETA : printf("-> "); break;
        case SHL : printf("<< "); break;
        case SHR : printf(">> "); break;

       }
}





