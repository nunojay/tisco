
#include "tipos.h"



/***************************************************************************/
/** CODIGO PARA MANIPULAR LISTAS DE DADOS **********************************/
// Manipulate data lists

#define ins_tipo(__tipo)      lst_tipos = ins_ele_lst(lst_tipos, __tipo)
#define prc_tipo(nom, tip)  \
        {  TITEM   _ele;                  \
           _ele.nome = nom;               \
           tip = proc_ele_lst(ini_lst(lst_tipos), &_ele, proc_item_fn);  }

#define ins_func(func)       lst_funcs = ins_ele_lst(lst_funcs, func)
#define prc_func(nom, fn)   \
        {  TFUNCAO   ele;               \
           ele.nome = nom;              \
           fn = proc_ele_lst(ini_lst(lst_funcs), &ele, proc_func_fn);  }

#define ins_lst(lst, ele)       lst = ins_ele_lst(lst, ele)

#define prc_item(lst, nom, item)  \
        {  TITEM   _ele;                  \
           _ele.nome = nom;               \
           item = proc_ele_lst(ini_lst(lst), &_ele, proc_item_fn);  }



int proc_item_fn (TITEM *a, TITEM *b);
int proc_func_fn (TFUNCAO *a, TFUNCAO *b);



void *push (TPLST *pilha, void *data);
void *pop (TPLST *pilha);



/**************************************************************************/
/** EXPRESSOES                                                           **/

TEXPR *make_expr_bin (TEXPR *es, TEXPR *d, texpr  oper);
TEXPR *make_expr_neg (TEXPR *ex);
TEXPR *make_expr_expr (TEXPR *ex);
TEXPR *make_expr_addr (TPLST var);
TEXPR *make_expr_konst (int val);
TEXPR *make_expr_var (TPLST var);
TEXPR *make_expr_string (char *st);

/**************************************************************************/
/** TIPOS


/**************************************************************************/
/** VARIAVEIS                                                            **/

TVAR *make_var_var (char *nome);
TVAR *make_var_stru (char *nome);
TVAR *make_var_seta (char *nome);
TVAR *make_var_asts (int nasts);
TVAR *make_var_subs (TPEXPR ex);
TVAR *make_var_call (char *nome, TPLST args);
TVAR *make_var_string (char *st);


/**************************************************************************/
/** CONDICOES                                                            **

/**************************************************************************/
/** INSTRUCOES                                                           **

/**************************************************************************/
/** LISTA FUNCOES                                                        **

/**************************************************************************/
/**                                                                      **


/****/



