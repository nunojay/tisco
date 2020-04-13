
  /*> parser.c <*/

  /* Parses the code and builds a tree representation.
   * This is a predictive parser.
   * Entry point in this module is function
   *    int parse_tic (char *dummy, int TokenFinal)
   * at the end.
   */



#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <setjmp.h>
#include "tipos.h"
#include "ticlex.h"
#include "rotaux.h"



TPLST parse_variavel ();
TPLST parse_lista_variaveis ();
TEXPR *parse_expr ();
TPLST parse_instrucoes ();
TPLST parse_lista_expressoes ();
TPLST parse_bloco ();
int proc_item_fn (TITEM *a, TITEM *b);



extern TLST  *lst_tipos;     // List of types
extern TLST  *lst_gvars;     // List of global vars
extern TLST  *lst_funcs;     // List of funcs
extern TLST  *lst_strings;   // List of strings
extern TDLL  *lst_dlls;      // List of DLLs used

static TFUNCAO  *funcao;     // Function beeing currently analised



/***************************************************************************/

TTOKEN Token, *ProxToken;       // (Also exported to exprs.c)
ttoken lookahead;               // (Also exported to exprs.c)
static jmp_buf  j;


void Erro (int cod, char *msg)
{
    printf("(%s L:%i C:%i): %s\n", filep->nome, filep->nlin,filep->ncol, msg);
	exit(cod); //longjmp(j, cod);
}


void match (ttoken token)
{
	if (lookahead == token)  {
		Token = *ProxToken;
		ProxToken = gettoken();
		lookahead = ProxToken->token;
	}
	else {
        TTOKEN  t = {token};
        printf("(%s L:%i C:%i): ", filep->nome, filep->nlin, filep->ncol);
        printt(&t);  printf(" expected.\n");
    	exit(-2); //	    longjmp(j, -2);
    }
}


void unmatch ()
{
	ungettoken(ProxToken);
	ProxToken = &Token;
	lookahead = ProxToken->token;
}


/***************************************************************************/
/***************************************************************************/



TPLST parse_variavel ()
/* Executes the parsing of a variable, and returns a list with its    *
 * components. Identifies strings, func calls and 'normal' variables. */
{
    TPLST  lv = NULL;

    if (lookahead == STRING)  {
       match(STRING);
       ins_lst(lv, make_var_string(Token.u.lexema));
       return lv;
    }

    if (lookahead == '*')  {
       int  nasts = 0;
       for (; (lookahead == '*'); nasts++)   match('*');
       ins_lst(lv, make_var_asts(nasts));
    }

    match(ID);
    /* Check calls to functions */
    if (lookahead == '(')  {
       TVAR   *v = make_var_call(Token.u.lexema, NULL);
       match('(');
       if (lookahead != ')')  v->u.call.args = parse_lista_expressoes();
       else                   v->u.call.args = NULL;
       match(')');
       ins_lst(lv, v);
       return lv;
    }

    ins_lst(lv, make_var_var(Token.u.lexema));
    while (1)  {
       switch (lookahead)  {
          case SETA :
               match(SETA);   match(ID);
               ins_lst(lv, make_var_seta(Token.u.lexema));
               continue;
          case '.' :
               match('.');    match(ID);
               ins_lst(lv, make_var_stru(Token.u.lexema));
               continue;
          case '[' :
               match('[');
               ins_lst(lv, make_var_subs( parse_expr() ));
               match(']');
               continue;
       }
       break;
    }

//    print_vars(ini_lst(lv));
    return ini_lst(lv);
}



TCOND *parse_condicao (ttoken Termin)
/* Parses a condition and returns its tree representation. 'Termin'  *
/* is the token that must follow the condition. It is used to make a *
 * distinction between a 'normal' condition and one which has only 1 item */
{
    TCOND  *c = New(TCOND);

    c->e = parse_expr();       // Get the condition's left side

    if (lookahead == Termin)  {      // Termin is the one who ends the cond
       c->d = NULL;
       c->oper = orCBOOL;       // Signals that it is just an item
       return c;
    }
    match(lookahead);     // Operador relacional
    switch (Token.token)  {
        case IGUAL :        c->oper = orIGUAL;         break;
        case DIFERENTE :    c->oper = orDIFERENTE;     break;
        case MAIOR :        c->oper = orMAIOR;         break;
        case MENOR :        c->oper = orMENOR;         break;
        case MAIOROUIGUAL : c->oper = orMAIOROUIGUAL;  break;
        case MENOROUIGUAL : c->oper = orMENOROUIGUAL;  break;
        default :
             Erro(-3, "Unknown relational operator");
    }
    c->d = parse_expr();       // Get the cond's right side
    return c;
}



/***************************************************************************/


// This macro returns != 0 iif t (token) is a language basic type
#define TIPO_BASICO(t)   \
    ( (t == CHAR) || (t == UCHAR) || (t == INT) || (t == UINT) || (t == VOID) )


int e_tipo ()
/* Returns true iif the next token is a type (basic or user defined) */
{
    if (TIPO_BASICO(lookahead))
       return 1;
    if (lookahead == ID)  {
       TITEM  *a;
       prc_tipo(ProxToken->u.lexema, a);
       if (a)  return 1;
    }
    return 0;
}

int e_funcao ()
/* Returns true iif the next token is a func */
{
    if (lookahead == ID)  {
       TFUNCAO  *a;
       prc_func(ProxToken->u.lexema, a);
       if (a)  return 1;
    }
    return 0;
}



enum grpt { COMVOID, SEMVOID };
typedef enum grpt grpt;


TTIPO *parse_tipo_base (grpt grp)
/* Returns a TTIPO* w/ the base type of variable/type declaration. The base type is the 1st
   token on a variable/type declaration, such as 'int' in 'int a;' or 'TPINT' in 'typedef TPint *int'.
   grp == COMVOID if the void type is aceptable, SEMVOID otherwise.
*/
/* Retorna um TTIPO* com o tipo base de uma declaracao de vars ou tipos. *
 * O tipo base e' o primeiro token que aparece numa declaracao de var ou *
 * tipo, como 'int'  em 'int  a;' ou 'TPINT' em 'typedef TPINT  *int'.
 * grp == COMVOID se o tipo void e' aceitavel, SEMVOID c.c. */
{
    TTIPO  *t = New(TTIPO);
    TITEM  *it;

    switch (lookahead)  {
         case ID :  free(t);
                   // Procurar o tipo (pode ser o proprio)    (look for the type (may be itself))
                    prc_tipo(ProxToken->u.lexema, it);
                    if (!it)  Erro(-5, "Type undeclared");
                    t = it->tipo;
                    break;
         case VOID :  if (grp == SEMVOID)  Erro(-90, "void can't be used here");
                      t->tipo = tVOID;  t->size = 0;  break;
         case CHAR :  t->tipo = tCHAR;  t->size = 1;  break;
         case UCHAR : t->tipo = tUCHAR; t->size = 1;  break;
         case INT :   t->tipo = tINT;   t->size = 2;  break;
         case UINT :  t->tipo = tUINT;  t->size = 2;  break;
         default : Erro(-2, "Invalid base type");
    }
    match(lookahead);
    return t;
}


TTIPO *parse_asts (TTIPO *t)
/* Looks for an asterisk, and modifies the t type description accordingly. Return the new type, is changed. */
/* Procura um asterisco, modificando de acordo a descricao do tipo t. *
 * Retorna o (possivelmente) novo tipo.                               */
{
    while (lookahead == '*')  {
       TTIPO  *aux;
       match('*');
      // Colocar o novo elemento 'a cabeca da cadeia    (put the new element at the head)
       aux = t;
       t = New(TTIPO);
       t->tipo = tPONT;
       t->size = 2;         // Tamanho de um ponteiro (16 bits)     (a pointer's size, 16 bits)
       t->u.tipo = aux;
    }
    return t;
}


TTIPO *parse_item (TTIPO *t, char **nome_item)
/* Parses an item's declaration, removing the base type. t has a ariable declaration base type. */
/* Efectua o parsing a uma declaracao de item, retirado o tipo base. *
 * t contem o tipo base das declaracoes de variaveis.                */
{
    TTIPO  *aux;

    t = parse_asts(t);

   // Obter o novo ID    (get the new ID)
    match(ID);
    *nome_item = Token.u.lexema;

   // Ver se ha' subscritos     (check if there are any indexes)
    if (lookahead == '[')  {
       TEXPR   *expr;
       match('[');
       aux = t;
       t = New(TTIPO);
       t->tipo = tARRAY;
       t->u.array.tipo_elem = aux;
       expr = parse_expr();       // Tem que ser uma expressao constante     (must be a constant expression)
       if (expr->oper != eKONS)
          Erro(-6, "An array size must be constant");
       t->u.array.nelem = expr->u.k;   //calc_exprk(expr);
       t->size = t->u.array.nelem * aux->size;   // Tamanho do tipo    (a type's size)
       free(expr);        //!!!!! NAO CHEGA!!! e' uma estrutura complexa ******    (free isn't enough to free the entire memory)
       match(']');
    }

    return t;
}




void parse_decl_item (TPLST *lst, classe_item classe)
/* Item declaration parsing. It's used parsing local/global variable declarations,
   formal function arguments, data structure fields and types.
   Parses phrases of the kind: type  [*] ID [[expr]] [, [*] ID ... ]
   lst references a lsit where the items will be inserted. No parsing of ';'
   at the end since it doesn't always end with ';'.
 */
/* Executa o parsing de uma declaracao de um item. E' usado no parsing de *
 * declaracoes de vars globais, locais, argumentos formais de funcoes,    *
 * campos de estruturas e tipos.                                          *
 * Faz parsing a frases do tipo:  tipo  [*] ID [[expr]] [, [*] ID ... ]   *
 * lst referencia uma lista onde serao inseridos os items.                *
 * No final nao se faz o match de ';' pq nem sempre termina com ';'.      */
{
    TITEM   *it;
    TTIPO   *tipo_base;

   // Obter o tipo base (o tipo comum) de todos os items    (get the base (common) type of all items)
    tipo_base = parse_tipo_base (SEMVOID);
   // Obter os items
    do {
        it = New(TITEM);
        it->classe = classe;
       // Obter o tipo complementar e o nome    (get the complementary type and the name)
        it->tipo = parse_item(tipo_base, &it->nome);
       // Ver se o nome ja' existe na lista     (check if the name already exists)
        if (proc_ele_lst(ini_lst(*lst), it, proc_item_fn))
           Erro(-19, "Duplicated name");
       // Inserir o novo item na lista     (insert the new name in the list)
        *lst = ins_ele_lst(*lst, it);
        if (lookahead == ',')
           match(',');
    } while (Token.token == ',');
}




int calc_stru_size (TPLST cs)
/* Looks up the fields list, and returns the total size of the fields. */
/* Pesquisa a lista de campos, e retorna o tamanho total dos campos. */
{
    int  size = 0;
    for (cs; cs; cs = cs->prox)  {
        TTIPO  *t = ((TITEM*)cs->data)->tipo;
        size += t->size;
    }
    return size;
}


void definicao_tipo ()
/* Parses a new type and inserts in to the types list. */
/* Efectua o parsing de um novo tipo e insere-o na lista de tipos */
{
    TITEM  *it = New(TITEM);

    match(TYPEDEF);
    it->classe = ciTIPO;
    if (lookahead == STRUCT)  {       // Estruturas  (structures)
        match(STRUCT);
        it->tipo = New(TTIPO);
        it->tipo->tipo = tSTRU;
        match(ID);            // Nome do tipo
        it->nome = Token.u.lexema;
        it->tipo->u.campos = NULL;
        match('{');
        it->tipo->size = 0;
       // Insert the new type, so that it is available in the type list immediatelly,
       // sinze we may have to used, if there's a pointer to itself.
       // Inserir o novo tipo na lista de tipos. Inseri-lo ja' para estar
       // desde ja' disponivel, porque pode ser preciso se houver nesta
       // estrutura um ponteiro para o proprio, caso em que havera' uma
       // procura por este tipo aquando do parsing do campo pont. p/ proprio.
        ins_tipo(it);

       // Obter os campos da estrutura     (get the struct fields)
        while (lookahead != '}')  {
            parse_decl_item(&it->tipo->u.campos, ciCAMPO);
            match(';');
        }
        match('}');
        it->tipo->u.campos = ini_lst(it->tipo->u.campos);
        it->tipo->size = calc_stru_size(it->tipo->u.campos);
    }
    else  {                           // Tipos simples     (simple types)
       // Obter o tipo complementar e o nome do tipo    (get the complementary type and the type's name)
        it->tipo = parse_item(parse_tipo_base(SEMVOID), &it->nome);
       // Ver se o nome ja' existe na lista     (check if the ame already exists in the list)
        if (proc_ele_lst(ini_lst(lst_tipos), it, proc_item_fn))
           Erro(-19, "Duplicated name");
        match(';');
        ins_tipo(it);          // Inserir o novo tipo na lista de tipos    (insert the new type in the types list)
    }
}





void parse_args_formais (TPLST *lst)
/* Parse a function's formal arguments. */
/* Executa o parsing dos argumentos formais de uma funcao */
{
    TITEM   *it;
   // Obter os argumentos    (get the arguments)
    do {
        it = New(TITEM);
        it->classe = ciARG;
       // Obter o tipo complementar e o nome da variavel    (get thte variable's complementary type)
        it->tipo = parse_item(parse_tipo_base (SEMVOID), &it->nome);
       // Ver se o nome ja' existe na lista    (check if the name already eixsts in the list)
        if (proc_ele_lst(ini_lst(*lst), it, proc_item_fn))
           Erro(-19, "Duplicated name");
       // Inserir o nome    (isnert the name)
        *lst = ins_ele_lst(*lst, it);
        if (lookahead == ',')
           match(',');
    } while (Token.token == ',');
}



TATTR *parse_atribuicao ()
// Parse an assignment, and return a tree of it.
/* Faz o parsing de uma atribuicao, retornando uma arvore para a mesma. */
{
    TATTR *a = New(TATTR);
    a->lvalue = parse_variavel();
    switch (lookahead)  {
       case '=' :  match('=');  a->rvalue = parse_expr();  break;
       case MAISMAIS :
       case MENOSMENOS :
            a->rvalue = make_expr_bin(make_expr_var(a->lvalue),
                                      make_expr_konst(1),
                                      ((lookahead == MAISMAIS)? eADD: eSUB) );
            match(lookahead);
            break;
       default : Erro(-29, "=, ++ or -- expected");
    }
    return a;
}



TINSTR *parse_instrucao ()
{
    TINSTR *i;

    if ( (lookahead == ';') || (lookahead == '}')  )  {  // ('}' qdo o bloco e' vazio)    ('}' when the block is empty)
       match(';');     // Instrucao vazia        (empty instruction)
       return NULL;
    }

    i = New(TINSTR);
    // File name and line number of the statment.
    // Nr da linha e nome do file fonte da instrucao (posicao da instr)
    memcpy(&i->posi, filep, sizeof(TFINFO));

    switch (lookahead)  {
       case RETURN :
            i->itipo = iRETURN;
            match(RETURN);
            if (lookahead != ';')    i->u.expr_ret = parse_expr();
            else                     i->u.expr_ret = NULL;
            match(';');
            break;

       case BREAK :
            i->itipo = iBREAK;
            match(BREAK);  match(';');
            break;

       case SWITCH : {
            TCASELST   *casel;
            TEXPR      *ex;

            i->itipo = iSWITCH;
            match(SWITCH);
            match('(');
            i->u.swi.valor = parse_expr();
            match(')');  match('{');

            i->u.swi.Default = NULL;
            for (i->u.swi.cases = NULL; lookahead != '}';)  {
                 if (lookahead == DEFAULT)  {
                    match(DEFAULT);  match(':');
                    i->u.swi.Default = parse_instrucoes();
                    break;
                 }

                 match(CASE);
                 casel = New(TCASELST);
                 ex = parse_expr();
                 if (ex->oper != eKONS)
                    Erro(-39, "'case' value must be 8bit constant");
                 else if ((ex->u.k < -128) || (ex->u.k > 255))
                    Erro(-39, "'case' value must be 8bit constant");
                 match(':');
                 casel->tval = ex->u.k;
                 casel->bloco = parse_instrucoes();
                 i->u.swi.cases = eins_ele_lst(i->u.swi.cases, casel);
            }
            match('}');
            i->u.swi.cases = ini_lst(i->u.swi.cases);
            break; }

       case FOR :
            i->itipo = iFOR;
            match(FOR);  match('(');
            if (lookahead != ';')   i->u.ifor.aini = parse_atribuicao();
            else                    i->u.ifor.aini = NULL;
            match(';');
            if (lookahead != ';')   i->u.ifor.cond = parse_condicao(';');
            else                    i->u.ifor.cond = NULL;
            match(';');
            if (lookahead != ')')   i->u.ifor.aciclo = parse_atribuicao();
            else                    i->u.ifor.aciclo = NULL;
            match(')');
            i->u.ifor.corpo = parse_instrucoes();
            break;

       case WHILE :
            i->itipo = iFOR;
            match(WHILE);  match('(');
            i->u.ifor.aini = NULL;
            i->u.ifor.cond = parse_condicao(')');
            i->u.ifor.aciclo = NULL;
            match(')');
            i->u.ifor.corpo = parse_instrucoes();
            break;

       case ASM :  {
            char  buf[16384];
            i->itipo = iASM;
            match(ASM);

            if (lookahead == '{')  {
               lookstr(buf, "};", 16384);
               if (buf[strlen(buf)-2] != '}')
                  if (buf[strlen(buf)-1] != ';')
                     Erro(-23, "'};' expected or asm code too big (16Kb max text size)");
               buf[strlen(buf)-2] = 0;
               i->u.iasm = dupstr(buf);
               ProxToken = gettoken();
               lookahead = ProxToken->token;
            }
            else if (lookahead == STRING)  {
               char  *s;
               match(STRING);
               i->u.iasm = dupstr(Token.u.lexema);
               // Eliminar \ de \"    (delete the backslash)
               s = i->u.iasm;
               while (s = strstr(s, "\\\""))
                     strcpy(s, s+1);
               match(';');
            }
            else
               Erro(-23, "'{' or \" expected");
            } break;

       case IF :
            i->itipo = iIF;
            match(IF);  match('(');
            i->u.iif.cond = parse_condicao(')');
            match(')');
           // Parse do corpo directo do if      (direct 'if' statment body parsing)
            i->u.iif.corpo = parse_instrucoes();
           // Parse do corpo else do if, se o houver    (else part parsing, if it exists)
            i->u.iif.alter = NULL;
            if (lookahead == ELSE)  {
               match(ELSE);
               i->u.iif.alter = parse_instrucoes();
            }
            break;

       case IFC : case IFNC : case IFZ : case IFNZ :
            switch (lookahead)  {
                case IFC  : i->itipo = iIFC;  break;
                case IFNC : i->itipo = iIFNC; break;
                case IFZ  : i->itipo = iIFZ;  break;
                case IFNZ : i->itipo = iIFNZ; break;
            }
            match(lookahead);
            i->u.iif.cond = NULL;
           // Parse do corpo directo do if    (parse true condition block)
            i->u.iif.corpo = parse_instrucoes();
           // Parse do corpo else do if, se o houver     (parse else condition block, if any)
            i->u.iif.alter = NULL;
            if (lookahead == ELSE)  {
               match(ELSE);
               i->u.iif.alter = parse_instrucoes();
            }
            break;

       case GETE :
            i->itipo = iGETE;
       case GETV :
            if (lookahead == GETV)  i->itipo = iGETV;
            match(lookahead);
            i->u.var = parse_variavel();
            match(';');
            break;

       default :     // atribuicoes/chamadas/decl_vars_locais    (assigns/calls/local-vars declaration)
            if (e_tipo())  {   // Declaracao de var local    (local variable declaration)
               free(i);
               i = NULL;
               parse_decl_item(&funcao->locais, ciLOCAL);
               match(';');
            }
            else if (e_funcao())  {   // Chamada a uma funcao    (function call)
               i->itipo = iCALL;
               match(ID);
               i->u.call.nome_fn = Token.u.lexema;
               match('(');
              // Obter os parametros actuais da funcao, se os houver    (get the actual function arguments, if any)
               i->u.call.args = NULL;
               if (lookahead != ')')
                  i->u.call.args = parse_lista_expressoes();
               match(')');
               match(';');
            }
            else  {     // E' atribuicao    (it's an assigment)
               i->itipo = iATTR;
               i->u.iattr = parse_atribuicao();
               match(';');
            }
    }
    return i;
}


TPLST parse_bloco ()
/* Parses a block and returns an instruction list */
/* Faz parsing a um bloco e retorna uma lista de instrucoes */
{
    TPLST  cor = NULL;
    TINSTR  *i;

    // Insert an empty instruction, to distinguish between a function w/ an empty body and an external function.
    // Inserir 1 instr vazia, para destinguir entre uma funcao com o corpo
    // vazio e outra que e' uma funcao externa
    i = New(TINSTR);
    memset(&i->posi, 0, sizeof(TFINFO));
    i->itipo = iNULA;
    cor = ins_ele_lst(cor, i);

    match('{');

    while (1)  {
       if ((lookahead != EOT) && (lookahead != '}'))  {
           i = parse_instrucao();
           if (i)
              cor = ins_ele_lst(cor, i);
           continue;
       }
       break;
    }
    match('}');
    cor = ini_lst(cor);
    return cor;
}


TPLST parse_instrucoes ()
// Parses an instruction or block.
/* Faz o parsing a uma instrucao ou um bloco */
{
    if (lookahead == '{')
       return parse_bloco();
    else
       return ins_ele_lst(NULL, parse_instrucao());
}



void parse_funcao ()
{
    TFUNCAO *fn;

    funcao = New(TFUNCAO);  // 'funcao' e' var global      ('funcao' is global variable)
    funcao->flags = 0;      // Init o campo flags          (initialize the flags field)
    funcao->tipo_ret = parse_asts(parse_tipo_base(COMVOID));
    match(ID);
    funcao->nome = Token.u.lexema;
    // Ver se ja' existe uma funcao (nao prototipo) com o mesmo nome    (check if there's a non-prototype function w/ the same name)
    fn = proc_ele_lst(ini_lst(lst_funcs), funcao, proc_func_fn);
    if (fn)  {
       if (fn->corpo)
          Erro(-19, "Duplicated function name");
    }

    match('(');
    // Obter os argumentos da funcao, se os houver   (get the function's arguments, if any)
    funcao->locais = NULL;
    if (lookahead != ')')
       parse_args_formais(&funcao->locais);
    match(')');
    // Verificar se existe a keyword 'force'     (is there a 'force' keyword?)
    if (lookahead == FORCE)  {
        match(FORCE);
        SETfnFORCE(*funcao);    // Forcar esta funcao no output   (always output the function's code)
    }
    // Verificar se existe a keyword 'export'    (is there an 'export' keyword?)
    if (lookahead == EXPORT)  {
        match(EXPORT);
        SETfnEXPORT(*funcao);
        match(INTEIRO);
        funcao->index = Token.u.i;
    }
    // Insert the function now in the functions list, before parsing the body, to support recursivity.
    // Inserir ja' a funcao na lista, antes de se fazer o
    // parsing ao seu corpo, de modo a poder ser chamada
    // dentro dela propria.
    ins_func(funcao);    // Inserir a nova funcao na lista
    // Obter o corpo da funcao, se nao for um prototipo     (get the function's body, if not a prototype)
    if (lookahead == ';')  {
       match(';');
       funcao->corpo = NULL;
    }
    else
       funcao->corpo = ini_lst(parse_bloco());
    // (So' agora, depois do parse das vars locais no corpo da funcao)
    funcao->locais = ini_lst(funcao->locais);

    // Se havia um prototipo, elimina'-lo (e' + facil "estracalha'-lo"...)    (if there was a prototype, get rid of it)
    if (fn)  {
       *fn->nome = fn->nome[1] = '®';   // Para nao ser encontrado   (make it unfindable)
       fn->flags = 0;     // Para nao ser gerado     (make it not-generatable)
    }
}



void parse_decl_DLL ()
{
    extern TOPT  cmdl;
    TDLL   *dll;
    int    len;

    match(DLL);
    match(ID);

    // If we're compiling a DLL we look at a declaracion as prototypes.Allows use of the same DLL .h file
    // both in the DLL and client program.
    // Se estamos a compilar uma DLL simplesmente 'olha-se' para a
    // declaracao como prototipos. Permite tb usar o mesmo ficheiro .h
    // com a declaracao da DLL, contendo definicoes auxiliares, no codigo
    // da DLL e no programa que usa a DLL.
    if (cmdl.tipo_prog == tpDLL)  {
       match('{');
       while ((lookahead != '}') && (lookahead != EOT))  {
             parse_funcao();
            if (funcao->corpo != NULL)
               Erro(-17, "DLL functions prototypes don't have bodys");
       }
       match('}');
       return;
    }

    dll = New(TDLL);

    dll->nome = meulloc(16);
    len = strlen(Token.u.lexema+1);
    if (len < 8)
       sprintf(dll->nome, " \\00%i%s", len, Token.u.lexema+1);
    else if (len == 8)
       sprintf(dll->nome, " \\010%s", Token.u.lexema+1);
    else
       ErroF(-87, "DLL name too long (max: 8 chars)");
    free(Token.u.lexema);
    ins_lst(lst_strings, dll->nome);   // Para a lista de strings

    dll->protos = NULL;
    match('{');

    while (lookahead != '}')  {
       parse_funcao();
       if (funcao->corpo != NULL)
          Erro(-17, "DLL functions prototypes don't have bodys");
       dll->protos = ins_ele_lst(dll->protos, funcao);
    }
    match('}');

    dll->protos = ini_lst(dll->protos);
    lst_dlls = eins_ele_lst(lst_dlls, dll);
}


/**************************************************************************/


int testa_frase (ttoken token, int max_tokens)
// token lookahead, up to max_tokens ahead.
/* Testa se um token aparece no caudal de tokens, nos proximos max_tokens *
 * tokens. Retorna um booleano.                                           */
{
    TTOKEN  toks[12];
    int     ctr, res;
    ttoken  antes = lookahead;

    assert(max_tokens <= 12);
   // Ler alguns tokens procurando token
    for (res = 0, ctr = 0;  ctr < max_tokens;)
        if (lookahead != token)  {
           match(lookahead);
           toks[ctr++] = Token;
        }
        else  {
           res = 1;
           break;
        }
   // Repor os tokens lidos
    unmatch(lookahead);
    while (--ctr >= 0)
        ungettoken(&toks[ctr]);
    ProxToken = gettoken();
    lookahead = ProxToken->token;
    assert(antes == lookahead);
    return res;
}





void programa ()
{
    while (1)  {

        switch (lookahead)  {
            case EOT :
                 break;

          // Type definition
            case TYPEDEF :
                 definicao_tipo();
                 continue;

            case DLL :
                 parse_decl_DLL();
                 continue;

          // Funcs and vars declaration
            default :
                 if (testa_frase('(', 4))  {  // Funcao
                    parse_funcao();
                 }
                 else  {  // Variaveis globais
                    if (lookahead == EXTERN)  {
                       match(lookahead);
                       parse_decl_item(&lst_gvars, ciGLBEXTRN);
                    }
                    else
                       parse_decl_item(&lst_gvars, ciGLOBAL);
                    match(';');
                 }
                 continue;
        }

        break;
    }
}







/***************************************************************************/
/***************************************************************************/


int parse_tic (char *dummy, int TokenFinal)
// Neste momento esta rotina retorna sempre 0
{
   int  r;

	if (r = setjmp(j))
	   return r;                // Never reach here

	ProxToken = gettoken();
	lookahead = ProxToken->token;

	programa();

	if (TokenFinal)
		match(TokenFinal);

   // The result of programa() is 5 lists (in global variables):
   //      lst_tipos - list of types
   //      lst_gvars - list of global variables (vars) (static vars)
   //      lst_funcs - list of functions
   //      lst_strings - list of strings
   //      lst_dlls - list of used DLLs
    lst_tipos = ini_lst(lst_tipos);
    lst_gvars = ini_lst(lst_gvars);
    lst_funcs = ini_lst(lst_funcs);
    lst_strings = ini_lst(lst_strings);
    lst_dlls = ini_lst(lst_dlls);

//    printf("Tipos:\n ");          print_items(lst_tipos, 1);
//    printf("Vars Globais:\n ");   print_items(lst_gvars, 1);
//    printf("Funcoes:\n ");        print_funcs(lst_funcs, 1);

	return 0;
}




