
  /*> exprs.c <*/

  /*  Has functions related with expressions.
   *  This module's exported functions are:
   *
   *    int calc_exprk (TEXPR *ex)          (calc const expressions)
   *    TEXPR *parse_expr ()
   *    TPLST parse_lista_expressoes ()     (parse expression list)
   */


#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "tipos.h"
#include "ticlex.h"
#include "rotaux.h"


void Erro (int cod, char *msg);
TPLST parse_variavel ();


extern TLST  *lst_strings;         // Lista de strings

extern TTOKEN Token, *ProxToken;
extern ttoken lookahead;




int calc_exprk (TEXPR *ex)
// Evaluates 'ex' assuming that it is a constant expression and returns its value.
// Used for example to calculate array size on their declarations.
/* Avalia ex assumindo que e' uma expressao constante e retorna o seu valor. */
/* Usada (por ex.) para calcular a dimensao de arrays na sua declaracao. */
{
    assert(ex != NULL);
    switch (ex->oper)  {
       case eEXPR : return calc_exprk(ex->u.expr);
       case eADD  : return calc_exprk(ex->u.bin.e) + calc_exprk(ex->u.bin.d);
       case eSUB  : return calc_exprk(ex->u.bin.e) - calc_exprk(ex->u.bin.d);
       case eMUL  : return calc_exprk(ex->u.bin.e) * calc_exprk(ex->u.bin.d);
       case eDIV  : return calc_exprk(ex->u.bin.e) / calc_exprk(ex->u.bin.d);
       case eMOD  : return calc_exprk(ex->u.bin.e) % calc_exprk(ex->u.bin.d);
       case eAND  : return calc_exprk(ex->u.bin.e) & calc_exprk(ex->u.bin.d);
       case eOR   : return calc_exprk(ex->u.bin.e) | calc_exprk(ex->u.bin.d);
       case eXOR  : return calc_exprk(ex->u.bin.e) ^ calc_exprk(ex->u.bin.d);
       case eSHL  : return calc_exprk(ex->u.bin.e) << calc_exprk(ex->u.bin.d);
       case eSHR  : return calc_exprk(ex->u.bin.e) >> calc_exprk(ex->u.bin.d);
//       case eNEG  : return -calc_exprk(ex->u.neg);
       case eKONS : return ex->u.k;
       default :  // eVAR  eADDR
            Erro(-4, "Invalid constant expression");
    }
}


static int expr (TPLST *pilha);
static int termo (TPLST *pilha);
static int factor (TPLST *pilha);
static int logico (TPLST *pilha);



#define CONSTROI_OP_BIN(regra, op, op_enum)    \
        {   TEXPR  *exa, *exb;            \
            match(lookahead);             \
            exa = pop(pilha);             \
            regra(pilha);                 \
            exb = pop(pilha);             \
           /* Se ambos os operandos forem constantes, fazer ja' a conta   (if both ooperands constat, do the calc now) */  \
            if ( (exa->oper == eKONS) && (exb->oper == eKONS) )  {      \
               if ( (exb->u.k == 0) && ((op_enum == eDIV) || (op_enum == eMOD)) )  \
                  Erro(-8, "Divide by 0");                              \
               exa->u.k = exa->u.k op exb->u.k;                         \
               free(exb);                                               \
               push(pilha, exa);                                        \
            }                                                           \
            else                                                        \
               push( pilha, make_expr_bin(exa, exb, op_enum) );         \
        }


void constroi_op_bin (TPLST *pilha, int (*regra)(TPLST*), texpr op_enum)
{   TEXPR  *exa, *exb;
    match(lookahead);
    exa = pop(pilha);
    regra(pilha);
    exb = pop(pilha);
   /* Se ambos os operandos forem constantes, fazer ja' a conta   (if both ooperands constat, do the calc now) */
    if ( (exa->oper == eKONS) && (exb->oper == eKONS) )  {
       if ( (exb->u.k == 0) && ((op_enum == eDIV) || (op_enum == eMOD)) )
          Erro(-8, "Divide by 0");
       switch (op_enum)   {
          case eADD : exa->u.k = exa->u.k + exb->u.k; break;
          case eSUB : exa->u.k = exa->u.k - exb->u.k; break;
          case eMUL : exa->u.k = exa->u.k * exb->u.k; break;
          case eDIV : exa->u.k = exa->u.k / exb->u.k; break;
          case eMOD : exa->u.k = exa->u.k % exb->u.k; break;
          case eAND : exa->u.k = exa->u.k & exb->u.k; break;
          case eOR : exa->u.k = exa->u.k | exb->u.k; break;
          case eXOR : exa->u.k = exa->u.k ^ exb->u.k; break;
          case eSHL : exa->u.k = exa->u.k << exb->u.k; break;
          case eSHR : exa->u.k = exa->u.k >> exb->u.k; break;
       }
       free(exb);
       push(pilha, exa);
    }
    else
       push( pilha, make_expr_bin(exa, exb, op_enum) );
}



TEXPR *parse_expr ()
// Parses an expression and builds a tree for it. Returns the tree.
/* Efectua o parsing a uma expressao e constroi uma arvore para ela. */
/* Retorna a arvore */
{
    TPLST  pilha = NULL;
    TEXPR  *e;

    if (lookahead == STRING)  {
       match(lookahead);
       e = make_expr_string(Token.u.lexema);
       ins_lst(lst_strings, Token.u.lexema);
    }
    else  {
       expr(&pilha);
       e = pop(&pilha);
    }
//      print_expr(e);printf("\n");
    return e;
}


static int expr (TPLST *pilha)
{
	termo(pilha);
	switch (lookahead)  {
	   case '+' : //CONSTROI_OP_BIN(expr, +, eADD);  break;
                  constroi_op_bin(pilha, expr, eADD); break;
	   case '-' : //CONSTROI_OP_BIN(expr, -, eSUB);  break;
                  constroi_op_bin(pilha, expr, eSUB); break;
	}
}


static int termo (TPLST *pilha)
{
	factor(pilha);
	switch (lookahead)  {
	   case '*' : //CONSTROI_OP_BIN(termo, *, eMUL);  break;
                  constroi_op_bin(pilha, termo, eMUL); break;
	   case '/' : //CONSTROI_OP_BIN(termo, /, eDIV);  break;
                  constroi_op_bin(pilha, termo, eDIV); break;
	   case '%' : //CONSTROI_OP_BIN(termo, %, eMOD);  break;
                  constroi_op_bin(pilha, termo, eMOD); break;
	}
}


static int factor (TPLST *pilha)
{
	logico(pilha);
	switch (lookahead)  {
	   case '&' : //CONSTROI_OP_BIN(factor, &, eAND);   break;
                  constroi_op_bin(pilha, factor, eAND); break;
	   case '|' : //CONSTROI_OP_BIN(factor, |, eOR);    break;
                  constroi_op_bin(pilha, factor, eOR); break;
	   case '^' : //CONSTROI_OP_BIN(factor, ^, eXOR);   break;
                  constroi_op_bin(pilha, factor, eXOR); break;
	   case SHR : //CONSTROI_OP_BIN(factor, >>, eSHR);  break;
                  constroi_op_bin(pilha, factor, eSHR); break;
	   case SHL : //CONSTROI_OP_BIN(factor, <<, eSHL);  break;
                  constroi_op_bin(pilha, factor, eSHL); break;
	}
}


static int logico (TPLST *pilha)
{
    TEXPR  *exa;    // Auxiliar para quando for preciso

	switch (lookahead)  {
	   case '(' :
	        match('(');   expr(pilha);
            exa = pop(pilha);
            if (exa->oper == eKONS)
               push(pilha, exa);
            else
               push(pilha, make_expr_expr(exa));
            match(')');
	        break;
	   case '*' :  case ID :
	        push(pilha, make_expr_var(parse_variavel()));
	        break;
	   case INTEIRO :
	        match(INTEIRO);
            push(pilha, make_expr_konst(Token.u.i));
	        break;
	   case CAR :
	        match(CAR);
            push(pilha, make_expr_konst(Token.u.c));
	        break;
	   case '-' :
	        match('-');  expr(pilha);
            exa = pop(pilha);
           // If the stack's expression is a constant, negates it without creating a new expression.
           // Se a expressao na pilha for uma constante, nega-a sem
           // criar uma nova expressao.
            if (exa->oper == eKONS)
               exa->u.k = - exa->u.k;
            else  {
               //exa = make_expr_neg(exa);
               exa = make_expr_bin(make_expr_konst(0), exa, eSUB);
             }
            push(pilha, exa);
	        break;
	   case '&' :
	        match('&');
            push(pilha, make_expr_addr(parse_variavel()));
	        break;
	   default :
			Erro(-2, "&, *, (, -, ID, INTEGER or CHAR expected"); break;
	}
}


TPLST parse_lista_expressoes ()
/* Parses an expression's list, separated by ','. It is called to  *
 * parse function calls parameters. Returns a list of expressions. */
{
    TPLST  le = NULL;

    while (1)  {
        le = ins_ele_lst(le, parse_expr());
        if (lookahead != ',')
           break;
        match(',');
    }

    return ini_lst(le);
}

