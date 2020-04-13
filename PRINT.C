
  /* Rotinas para imprimir o conteudo das estruturas de dados. */
  /* Essencialmente para DEBUG.                                */

  /* These routines printf some Tisco's data structures.
     They are used for debug */

#include <stdio.h>
#include <assert.h>
#include "listas.h"
#include "tipos.h"


void print_items (TPLST li, int nl);
void print_item (TITEM *i);
void print_tipos (TPLST lt, int nl);
void print_var (TVAR *v);
void print_vars (TPLST lv, int nl);
void print_func (TFUNCAO *f);
void print_funcs (TPLST lv, int nl);




void printcode (TCODE *ls)
{
    if ((!ls->lbl) && (!ls->op) && (!ls->dst) && (!ls->src) && (!ls->txt))
       printf("  >>>>> 100%% NULO <<<<<");

    if (ls->lbl)
       printf("\n%s:", ls->lbl);
    if (ls->op)
       printf("\t%-8s", ls->op);
    if (ls->dst)
       printf("%s", ls->dst);
    if (ls->src)
       if (ls->dst)
          printf(", %s", ls->src);
       else
          printf("%s", ls->src);
    if (ls->txt)
       printf("%s", ls->txt);
    else
       printf("\n");
}


/**************************************************************************/
/** EXPRESSOES                                                           **/

void print_expr (TEXPR *e)
{
    switch (e->oper)  {
        case eSTRING : printf("%s", e->u.string);  break;
        case eADD : print_expr(e->u.bin.e); printf("+");
                    print_expr(e->u.bin.d); break;
        case eSUB : print_expr(e->u.bin.e); printf("-");
                    print_expr(e->u.bin.d); break;
        case eMUL : print_expr(e->u.bin.e); printf("*");
                    print_expr(e->u.bin.d); break;
        case eDIV : print_expr(e->u.bin.e); printf("/");
                    print_expr(e->u.bin.d); break;
        case eMOD : print_expr(e->u.bin.e);
                    print_expr(e->u.bin.d); printf("%");break;
        case eAND : print_expr(e->u.bin.e); printf("&");
                    print_expr(e->u.bin.d); break;
        case eOR  : print_expr(e->u.bin.e); printf("|");
                    print_expr(e->u.bin.d); break;
        case eXOR : print_expr(e->u.bin.e); printf("^");
                    print_expr(e->u.bin.d); break;
        case eSHL : print_expr(e->u.bin.e); printf("<<");
                    print_expr(e->u.bin.d); break;
        case eSHR : print_expr(e->u.bin.e); printf(">>");
                    print_expr(e->u.bin.d); break;
        case eKONS: printf("%i", e->u.k); break;
        case eVAR : print_vars(e->u.var, 0);  break;
        case eEXPR: printf("("); print_expr(e->u.expr); printf(")");
                    break;
        case eADDR: printf("&"); print_expr(e->u.expr); break;

        default : exit(printf("Unknown item %i (print_expr)\n", e->oper));
    }
}


void print_expr_postfix (TEXPR *e)
{
    switch (e->oper)  {
        case eSTRING : printf("%s", e->u.string);  break;
        case eADD : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("+");break;
        case eSUB : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("-");break;
        case eMUL : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("*");break;
        case eDIV : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("/");break;
        case eMOD : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("%");break;
        case eAND : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("&");break;
        case eOR  : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("|");break;
        case eXOR : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("^");break;
        case eSHL : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf("<<");break;
        case eSHR : print_expr_postfix(e->u.bin.e);
                    print_expr_postfix(e->u.bin.d); printf(">>");break;
//        case eNEG : printf("-"); print_expr(e->u.neg); break;
        case eKONS: printf("%i", e->u.k); break;
        case eVAR : print_vars(e->u.var, 0);  break;
        case eEXPR: printf("("); print_expr_postfix(e->u.expr); printf(")");
                    break;
        case eADDR: printf("&"); print_expr_postfix(e->u.expr); break;

        default : exit(printf("Unknown item %i (print_expr)\n", e->oper));
    }
}



void print2_expr (TEXPR *e)
{
    switch (e->oper)  {
        case eSTRING : printf("%s", e->u.string);  break;
        case eADD : printf("(");print2_expr(e->u.bin.e); printf("+");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eSUB : printf("(");print2_expr(e->u.bin.e); printf("-");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eMUL : printf("(");print2_expr(e->u.bin.e); printf("*");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eDIV : printf("(");print2_expr(e->u.bin.e); printf("/");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eAND : printf("(");print2_expr(e->u.bin.e); printf("&");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eOR  : printf("(");print2_expr(e->u.bin.e); printf("|");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eXOR : printf("(");print2_expr(e->u.bin.e); printf("^");
                    print2_expr(e->u.bin.d); printf(")"); break;
        case eSHL : print_expr(e->u.bin.e); printf("<<");
                    print_expr(e->u.bin.d); break;
        case eSHR : print_expr(e->u.bin.e); printf(">>");
                    print_expr(e->u.bin.d); break;
//        case eNEG : printf("-"); print2_expr(e->u.neg); break;
        case eKONS: printf("%i", e->u.k); break;
        case eVAR : print_vars(e->u.var, 0);  break;
        case eEXPR: printf("("); print2_expr(e->u.expr); printf(")");
                    break;
        case eADDR: printf("&"); print2_expr(e->u.expr); break;
//        case eABREPAREN : case eFECHAPAREN : break;

        default : exit(printf("Unknown item %i (print2_expr)\n", e->oper));
    }
}

void print_operando (TEXPR *op)
{
    switch (op->oper)  {
        case eKONS: printf("%i", op->u.k); break;
        case eVAR : print_vars(op->u.var, 0);  break;
        case eADDR: printf("&"); print_operando(op->u.expr); break;
        case eSTRING : printf("%s", op->u.string);  break;
    }
}

void print_oper (TEXPR *e)
{
    switch (e->oper)  {
        case eADD : printf("+"); break;
        case eSUB : printf("-"); break;
        case eMUL : printf("*"); break;
        case eDIV : printf("/"); break;
        case eMOD : printf("/"); break;
        case eAND : printf("&"); break;
        case eOR  : printf("|"); break;
        case eXOR : printf("^"); break;
        case eSHL : printf("<<"); break;
        case eSHR : printf(">>"); break;
//        case eNEG : printf("-"); break;
        case eEXPR: printf("("); printf(")");
                    break;

        default : exit(printf("Unknown item %i (print_operando)\n", e->oper));
    }
}





/**************************************************************************/
/** TIPOS                                                                **/

void print_tipo (TTIPO *t)
{
    assert(t != NULL);
    switch (t->tipo)  {
        case tCHAR :  printf("char(1)");   break;
        case tUCHAR : printf("uchar(1)");  break;
        case tINT :   printf("int(2)");    break;
        case tUINT :  printf("uint(2)");   break;
        case tPONT :  print_tipo(t->u.tipo); printf("*(2)"); break;
        case tARRAY : print_tipo(t->u.array.tipo_elem);
                      printf("[%i](%i)", t->u.array.nelem, t->size); break;
        case tSTRU :  printf("%s", t->u.nome);
                      print_items(t->u.campos, 0);
                      printf("(%i)", t->size);
                      break;
        case tID :    printf("%s(%i)", t->u.nome, t->size); break;
        default : printf(" ### ttipo desconhecido (%i) ### ", t->tipo);
    }
}


void print_tipos (TPLST lt, int nl)
{
    print_lista(lt, print_tipo, nl);
}




void print_item (TITEM *i)
{
    printf("<");
    switch (i->classe)  {
       case ciTIPO :   printf("T"); break;
       case ciGLOBAL : printf("G"); break;
       case ciLOCAL :  printf("L"); break;
       case ciARG :    printf("A"); break;
       case ciCAMPO :  printf(""); break;

       default : exit(printf("Unknown item %i (print_item)\n", i->classe));
    }
    printf(">%s:", i->nome);
    print_tipo(i->tipo);
}

void print_items (TPLST li, int nl)
{
    print_lista(li, print_item, nl);
}


/**************************************************************************/
/** VARIAVEIS                                                            **/

void print_var (TVAR *v)
{
    int  a;
    assert(v != NULL);
    switch (v->tok)  {
        case vtSTRING : printf("%s", v->u.nome);
        case vtVAR :  printf("%s", v->u.nome); break;
        case vtASTS : for (a = 0; a < v->u.nasts; a++) printf("*"); break;
        case vtSETA : printf("->%s", v->u.nome);  break;
        case vtSUBS : printf("[..]"); break;
        case vtSTRU : printf(".%s", v->u.nome); break;
        case vtCALL : printf("%s(", v->u.call.nome_fn);
                      print_vars(v->u.call.args, 0); printf(")"); break;
        default : printf(" ### vartok desconhecido (%i) ### ", v->tok);
    }
}

void print_vars (TPLST lv, int nl)
{
    print_lista(lv, print_var, nl);
}



/**************************************************************************/
/** FUNCOES                                                              **/

void print_func (TFUNCAO *f)
{
    printf("%s:", f->nome);
    print_tipo(f->tipo_ret);
    printf("(");
    print_items(f->locais, 0);
    printf(")");
}

void print_funcs (TPLST lv, int nl)
{
    print_lista(lv, print_func, nl);
}



