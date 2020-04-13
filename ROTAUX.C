
/* Auxiliary routines */


#include <malloc.h>
#include "listas.h"
#include "rotaux.h"
#include "tipos.h"


void *meulloc(int);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Pilha generica */

void *push (TPLST *pilha, void *data)
{
    *pilha = ins_ele_lst(*pilha, data);
}

void *pop (TPLST *pilha)
{
  void  *valor = (*pilha)->data;

    *pilha = del_ele_lst(*pilha);
    return valor;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int proc_item_fn (TITEM *a, TITEM *b)
{    return strcmp(a->nome, b->nome) == 0;    }

int proc_func_fn (TFUNCAO *a, TFUNCAO *b)
{    return strcmp(a->nome, b->nome) == 0;    }



/**************************************************************************/
/** EXPRESSOES                                                           **/

TEXPR *make_expr_bin (TEXPR *es, TEXPR *d, texpr  oper)
{
    TEXPR  *e = New(TEXPR);
    e->oper = oper;
    e->u.bin.e = es;
    e->u.bin.d = d;
    return e;
}

/*TEXPR *make_expr_neg (TEXPR *ex)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eNEG;
    e->u.neg = ex;
    return e;
} */

TEXPR *make_expr_expr (TEXPR *ex)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eEXPR;
    e->u.expr = ex;
    return e;
}

TEXPR *make_expr_addr (TPLST var)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eADDR;
    e->u.var = var;
    return e;
}

TEXPR *make_expr_konst (int val)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eKONS;
    e->u.k = val;
    return e;
}

TEXPR *make_expr_var (TPLST var)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eVAR;
    e->u.var = var;
    return e;
}

TEXPR *make_expr_string (char *st)
{
    TEXPR  *e = New(TEXPR);
    e->oper = eSTRING;
    e->u.string = meulloc(16);
    sprintf(e->u.string, "ST%u", (unsigned int)st);
    return e;
}


/**************************************************************************/
/** VARIAVEIS                                                            **/

TVAR *make_var_var (char *nome)
{
    TVAR  *v = New(TVAR);
    v->tok = vtVAR;
    v->u.nome = nome;
    return v;
}

TVAR *make_var_stru (char *nome)
{
    TVAR  *v = New(TVAR);
    v->tok = vtSTRU;
    v->u.nome = nome;
    return v;
}

TVAR *make_var_seta (char *nome)
{
    TVAR  *v = New(TVAR);
    v->tok = vtSETA;
    v->u.nome = nome;
    return v;
}

TVAR *make_var_asts (int nasts)
{
    TVAR  *v = New(TVAR);
    v->tok = vtASTS;
    v->u.nasts = nasts;
    return v;
}

TVAR *make_var_subs (TPEXPR ex)
{
    TVAR  *v = New(TVAR);
    v->tok = vtSUBS;
    v->u.index = ex;
    return v;
}

TVAR *make_var_call (char *nome, TPLST args)
{
    TVAR  *v = New(TVAR);
    v->tok = vtCALL;
    v->u.call.nome_fn = nome;
    v->u.call.args = args;
    return v;
}

TVAR *make_var_string (char *st)
{
    TVAR  *v = New(TVAR);
    v->tok = vtSTRING;
    v->u.nome = meulloc(16);
    sprintf(v->u.nome, "ST%u", (unsigned int)st);
    return v;
}




