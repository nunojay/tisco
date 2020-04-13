
  /*> limpeza.c <*/

  /*  Not used code elimination.
   *  Analizes the code's tree and marks only the used routines, having
   *  the 'main' function has root.
   *  Has no efect if we are compiling a DLL.
   *
   *  'limpa_rotinas_nao_usadas' is the only exported function.
   *
   *  When getting here, we assume that the "usage flag" is already 0.
   */



#include <stdio.h>
#include <assert.h>
#include "tipos.h"
#include "listas.h"
#include "rotaux.h"         // (prc_func)





extern TLST  *lst_funcs;     // Lista de funcoes




#define PROX(l)     ((l_) = (l_)->prox)


static void check_expr (TEXPR *ex);
static void check_funcao (char *fname);




static void check_params (TPLST p)
{
    for (; p; p = p->prox)  {
        TEXPR  *e = (TEXPR*)gelst(p);
        check_expr(e);
    }
}

static void check_fn_call (TCALL *ca)
{
    TFUNCAO  *f;
    prc_func(ca->nome_fn, f);
    check_params(ca->args);
    check_funcao(ca->nome_fn);
}



static void check_toks (TPLST lv)
{
    for (; lv; lv = lv->prox)  {
        TVAR    *v = (TVAR*)lv->data;
        if (v->tok == vtSUBS)
            check_expr(v->u.index);
    }
}


static void check_var (TPLST lv)
{
    TVAR    *v;

   // Se e' uma string, trata ja' e sai
    v = (TVAR*)lv->data;
    if (v->tok == vtSTRING)
       return;

   // Se e' uma chamada, trata ja' e sai
    if (v->tok == vtCALL)  {
       check_fn_call(&v->u.call);
       return;
    }

    check_toks(lv);
}




static void check_operando (TEXPR *ex)
{
    switch (ex->oper)  {
       case eVAR  : check_var(ex->u.var);  // So' poe o addr em hl
                    return;
       case eADDR : check_var(ex->u.var);
                    return;
    }
}



static void check_expr (TEXPR *ex)
{
    if (ex->oper == eEXPR) {
       check_expr(ex->u.expr);
       return;
    }

    if E_OPER_BIN(ex->oper)  {
       check_expr(ex->u.bin.e);
       check_expr(ex->u.bin.d);
    }
    else
       check_operando(ex);
}


/** EXPRESSOES                                                             **/
/****************************************************************************/


static void check_atribuicao (TATTR *a)
{
    check_expr(a->rvalue);      // Puts the result in the stack
    check_var(a->lvalue);       // Makes hl = var addr
}


static void check_condicao (TCOND *c)
{
    check_expr(c->e);
    if (c->oper != orCBOOL)
       check_expr(c->d);
}


static int chvalido (char s)
{
   return isdigit(s) || isalpha(s) || (s == '_');
}


static void check_bloco (TPLST bloco)
{
    for (; bloco; bloco = bloco->prox)  {
        TINSTR   *i = (TINSTR*)bloco->data;

        if (i == NULL)  continue;     // Instrucao vazia

        switch (i->itipo)  {
           case iRETURN :
                if (i->u.expr_ret)
                   check_expr(i->u.expr_ret);
                break;

           case iFOR :
                if (i->u.ifor.aini != NULL)
                   check_atribuicao(i->u.ifor.aini);
                if (i->u.ifor.cond != NULL)
                   check_condicao(i->u.ifor.cond);
                check_bloco(i->u.ifor.corpo);
                if (i->u.ifor.aciclo != NULL)
                   check_atribuicao(i->u.ifor.aciclo);
                break;

           case iSWITCH : {
                TCASELST  *cl = i->u.swi.cases;
                check_expr(i->u.swi.valor);
                for (; cl; cl = cl->prox)
                    check_bloco(cl->bloco);
                break; }

           case iIF :
                check_condicao(i->u.iif.cond);
           case iIFC : case iIFNC : case iIFZ : case iIFNZ :
                check_bloco(i->u.iif.corpo);
                if (i->u.iif.alter != NULL)
                   check_bloco(i->u.iif.alter);
                break;

           case iATTR :
                check_atribuicao(i->u.iattr);
                break;

           case iCALL :
                check_fn_call(&i->u.call);
                break;

           case iASM :  {
                char  *s, *f, tmp;

                for (s = i->u.iasm; s = (char*)strstr(s, "call"); s = f)  {
                     s += 4;
                     for (;*s && !chvalido(*s); s++);  // Saltar espacos iniciais
                     if (!s)  continue;
                     // Find the ID's end
                     for (f = s; *f && chvalido(*f); f++);
                     // Get the function's name and check it
                     tmp = *f;    *f = 0;
                     check_funcao(s);
                     *f = tmp;
                }
                } break;

           case iBREAK :
           case iGETE :
           case iGETV :
           default :
        }
    }
}




static void check_funcao (char *fname)
/* Searches a func for function calls. */
/* Pesquiza uma funcao em busca de chamadas a outras funcoes. Se a funcao
 * nao tiver corpo nao se faz nada (e' uma funcao externa). Se a funcao ja'
 * esta' marcada para output tb nao se faz nada, pois ja' foi pesquizada e
 * poderia resultar em deadlock. a funcao e' pesquizada se nao estiver ja'
 * marcada para OUTPUT ou estiver a ser forcada.
 * As funcoes externas tb sao marcadas se forem usadas. */
{
    TFUNCAO   *func;

    prc_func(fname, func);
    if (func)
        if ( /*fnFORCE(*func) || */!fnOUTPUT(*func) )  {
           SETfnOUTPUT(*func);
           if (func->corpo)
              check_bloco(func->corpo);
        }
}


static void check_funcoes_forcadas ()
/* Pesquiza funcoes (que foram forcadas) em busca de chamadas
   a outras funcoes. */
{
    TFUNCAO   *func;
    TPLST     lst;

    for (lst = lst_funcs; lst; lst = lst->prox)  {
       func = (TFUNCAO*)lst->data;

       if (fnFORCE(*func))  {
          SETfnOUTPUT(*func);
          if (func->corpo)
             check_bloco(func->corpo);
       }
    }
}


static void check_funcoes_exportadas ()
/* Pesquiza funcoes (que foram exportadas) em busca de chamadas
   a outras funcoes. */
{
    TFUNCAO   *func;
    TPLST     lst;

    for (lst = lst_funcs; lst; lst = lst->prox)  {
       func = (TFUNCAO*)lst->data;

       if (fnEXPORT(*func))  {
          SETfnOUTPUT(*func);
          if (func->corpo)
             check_bloco(func->corpo);
       }
    }
}




void limpa_rotinas_nao_usadas (ttprog tipo_prog)
{
    switch (tipo_prog)  {
       case tpAPP : check_funcoes_forcadas();
                    check_funcao("_main");
                    break;

       case tpDLL : check_funcoes_forcadas();
                    check_funcoes_exportadas();
                    break;
    }
}






