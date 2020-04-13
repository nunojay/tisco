
  /*> geraZ80.c <*/

  /*  Generates (gens) z80 code from the program's tree representation.
   *  Module entry point:
   *   void gera_codigo_Z80 (TROTNUM *rotnum)
   *  at the end of the module.
   *
   *  Function's return values:
   *      8 bits: A
   *     16 bits: HL
   *
   *  Function call model:
   *     ix is like a stack's SP that holds local and argument vars.
   *     The stack grows up. ix always points to the 1st position (pos)
   *     next to the end of the current func's local vars area.
   *
   *      func: add    ix, size_of_locals
   *            ...
   *            push   ix         ; get locals base addr to hl
   *            pop    hl
   *            add    hl, -des
   *            ...
   *            add    ix, -size_of_locals
   *            ret
   *
   *
   *  The label for JP instructions is the 'src' field of TCODE struct.
   *  It was more coherent to put it in the 'dst' field, but there's the
   *  need to have the JP isolated, and it must be able to hold a cond
   *  so that in the final text it will appear the format "JP cond, label"
   *  since the format used to write instructions to the output asm file
   *  is "INSTR DST, SRC".
   */



#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include "tipos.h"
#include "ticlex.h"
#include "rotaux.h"
#include "listas.h"



extern TOPT    cmdl;     // Command (cmd) line options

extern TLST  *lst_tipos;     // Lista de tipos   (types list)
extern TLST  *lst_gvars;     // Lista de variaveis globais   (global variables list)
extern TLST  *lst_funcs;     // Lista de funcoes   (functions list)
extern TLST  *lst_strings;   // Lista de strings    (strings list)



static TFUNCAO  *Func;     // Funcao actualmente a ser gerada   (function whos's code is currently being generated)
static TROTNUM  RoNu;      // Records the aritmetic routines beeing used in the code
TCODE  *lst_code;          // This list will have the generated code





#define DEB(msg)         //emit(0,"%t", msg)


int gera_expr (TEXPR *ex, TINSTR *ei);
int proc_item_fn (TITEM *a, TITEM *b);
int reload_konst (TEXPR *ex, TCODE *i, int size_k, int size_outro, int n_operan, int push_tambem, TINSTR *ei);
static void gera_cast (TCODE **lst, int size_var, int size, int n_operan);




static void Erro (int cod, TINSTR *i, char *msg)
{
    if (i) printf("(%s L:%i):", i->posi.nome, i->posi.nlin);
    printf(" %s\n", msg);
    exit(cod);
}

static void Warning (TINSTR *i, char *msg)
{
    if (i) printf("WARNING (%s L:%i):", i->posi.nome, i->posi.nlin);
    printf(" %s\n", msg);
}

static void ErroS (int cod, TINSTR *i, char *msg, char *s)
{
    if (i) printf("(%s L:%i):", i->posi.nome, i->posi.nlin);
    printf(msg, s);
    printf("\n");
    exit(cod);
}

static void WarningS (TINSTR *i, char *msg, char *s)
{
    if (i) printf("WARNING (%s L:%i):", i->posi.nome, i->posi.nlin);
    printf(msg, s);
    printf("\n");
}


/***************************************************************************/

#define emitPOP(ll,r)            emit(ll,"%o%d", "pop", r)
#define emitPUSH(ll,r)           emit(ll,"%o%s", "push", r)
#define emitINC(ll,r)            emit(ll,"%o%d", "inc", r)
#define emitLDreg(ll,rd, rs)     emit(ll,"%c", "ld", rd, rs)
#define emitLDint(ll,rd, i)      emit(ll,"%o%d%is", "ld", rd, i)

#define PRI   0
#define SEG   1

#define PUSHNAO  0          // For use with reload_konst
#define PUSHTB   1

#define emitPUSH_OPERAN(ll, sz__)            \
                   if (sz__ == 1)            \
                      emitPUSH(ll,"af");     \
                   else                      \
                      emitPUSH(ll,"hl");

#define emitPOP_OPERAN(ll, sz__, no__)                       \
                   if (sz__ == 1)  {                         \
                       if (no__ == PRI)  emitPOP(ll,"af");   \
                       else              emitPOP(ll,"bc");   \
                   } else {                                  \
                       if (no__ == PRI)  emitPOP(ll,"hl");   \
                       else              emitPOP(ll,"de");   \
                   }


static char *RTab[2][2] = { "a", "b",  "hl", "de" };
static char *RTab1[2][2] = { "h", "l",  "d", "e" };




char *getlbl (uint i)
{
    static char b[128];
    sprintf(b, "L%u", i);
    return b;
}

char *getrlbl (uint i)
{
    static char b[128];
    sprintf(b, "R%u", i);
    return b;
}


uint getln ()
{
   static unsigned int k = 0;
   return k++;
}


char *get_addrix (int des)
{
    static char  str[32];
    if (des >= 0)   sprintf(str, "(ix + %i)", +des);
    else            sprintf(str, "(ix - %i)", -des);
    return str;
}


static void emit (TCODE **lst, char *fmt, ...)
/* Emits an instruction, putting it in an opcodes list. *
 * If lst == NULL, lst_code is used.                    */
{
    va_list  ap;
    char   *p, *sval;
    int    ival;
    TCODE  *co = New(TCODE);

    memset(co, 0, sizeof(TCODE));      // NULL everything

    va_start(ap, fmt);
    for (p = fmt; *p; p++)  {
        if (*p != '%')
           ErroI(-50, "emit - unknown CHARS");

        switch (*++p)  {
           case 'i' :   // src or dst as integer instead of text
                if (*++p == 's')
                   sprintf(co->src = meulloc(34), "%+i", va_arg(ap, int));
                else
                   sprintf(co->dst = meulloc(34), "%+i", va_arg(ap, int));
                break;
           case 's' :       // 's' as 'source'
                co->src = dupstr(va_arg(ap, char*));
                break;
           case 'd' :       // 'd' as 'destino'
                co->dst = dupstr(va_arg(ap, char*));
                break;
           case 'o' :       // 'o' as 'opcode'
                co->op = dupstr(va_arg(ap, char*));
                break;
           case 't' :     // 't' as 'extra text' (comment, but also used for asm)
                co->txt = dupstr(va_arg(ap, char*));
                break;
           case 'c' :       // complete instruction: op dest, src
                co->op  = dupstr(va_arg(ap, char*));
                co->dst = dupstr(va_arg(ap, char*));
                co->src = dupstr(va_arg(ap, char*));
                break;
           case 'l' :       // label
                co->lbl = dupstr(va_arg(ap, char*));
                break;
           default : ErroI(-51, "emit - unknown fmt");
        }
    }
    va_end(ap);
    if (lst)  {       // Insert the instr in the lst, at any point
       *lst = eins_ele_lst(*lst,  co);
    }
    else
       lst_code = eins_ele_lst(lst_code,  co);
}




char *RegPar (char *reg)
/* Returns 1 ptr to 1 str with the reg pair to which a reg belongs */
{
    if (strlen(reg) == 2)  return reg;
    switch (reg[0])  {
         case 'a' : return "af";
         case 'b' : case 'c' : return "bc";
         case 'd' : case 'e' : return "de";
         case 'h' : case 'l' : return "hl";
    }
}



/****************************************************************************/
/** EXPRESSOES  (Expressions)                                              **/

static void emit_load_gptr ()
/* Generates code to derreference a pointer. hl has the address of a pointer
   variable. Generate the code to load hl with the address pointed to by the
   pointer */
/* Gera codigo para 'seguir' um ponteiro. hl contem o endereco de uma *
 * variavel ponteiro. Gera-se codigo para carregar em hl o endereco   *
 * apontado pelo ponteiro.                                            */
{
   /* Used register c because it is free for sure. de, b and a may be in
      use (waiting for the operation's other operand) . */
   // Usa-se o registo C pq de certeza que nao esta' em utilizacao.
   // DE, B e A podem estar em utilizacao (a guardar o outro operando
   // da operacao.
    emitLDreg(0, "c", "(hl)");        // c = (hl)
    emitINC(0, "hl");                 // hl++
    emitLDreg(0, "h", "(hl)");        // h = (hl)
    emitLDreg(0, "l", "c");           // l = c
}


void check_size (int size, TINSTR *ei)
{
    if ((size > 2) || (size < 1))
       Erro(-67, ei, "Invalid variable acess - data type not 8/16 bits");
}


static int gera_push_params (TPLST fargs, TPLST p, TINSTR *ei)
/* Generated code to pass arguments to a routine.
   Returns the nr of bytes pushed to the ix stack. */
/* Gera codigo para passar parametros a uma rotina. *
 * Retorna o n. de bytes colocados no stack ix.     */
{
    int   b = 0, size;
    char  str[16];

    for (; p && fargs; p = p->prox, fargs = fargs->prox)  {
        TEXPR  *e = (TEXPR*)gelst(p);
        TITEM  *item = (TITEM*)gelst(fargs);
        if (item->classe != ciARG)
           Erro(-57, ei, "Wrong arguments to function");
        check_size(size = gera_expr(e, ei), ei);
        size = reload_konst(e, lst_code->ante, size, item->tipo->size, PRI, PUSHTB, ei);
        emitPOP(0, RegPar(RTab[size-1][0]));     // pop  af/hl
        gera_cast (&lst_code, size, item->tipo->size, PRI);
        if (item->tipo->size == 1)
           emitLDreg(0, get_addrix(b++), "a");
        else {
           emitLDreg(0, get_addrix(b++), "l");
           emitLDreg(0, get_addrix(b++), "h");
        }
    }
    if (!fargs && p)
       Warning(ei, "Wrong arguments to function");
    return b;
}

static int gera_fn_call (TCALL *ca, TINSTR *ei)
/* Generates a function call. Returns the size of the function's return type. */
/* Gera codigo para a chamada a uma funcao. Retorna o size do tipo de *
 * retorno da funcao.                                                 */
{
    TFUNCAO  *f;

    DEB("\t;call\n");
    prc_func(ca->nome_fn, f);
    gera_push_params(f->locais, ca->args, ei);
    /* If compiling a DLL and the function isn't external, put a label on the call. */
    // Coloca-se um label na chamada se estamos a compilar um DLL e se
    // a funcao chamada nao e' externa (nao tem corpo)
    if ((cmdl.tipo_prog == tpDLL) && (f->corpo))
       emit(0,"%l%o%d", getrlbl((uint)ca), "call", ca->nome_fn);
    else
       emit(0,"%o%d", "call", ca->nome_fn);
    return f->tipo_ret->size;
}




static TPLST procura_tok (TPLST lv, vartok tok)
{
    for (; lv; lv = lv->prox)  {
        TVAR  *t = (TVAR*)lv->data;
        if (t->tok == tok)
           return lv;
    }
    return NULL;
}


static int get_lvar_des (TFUNCAO *f, char *nome)
/* Returns the offset of a local variable in relation to ix inside a function. Includes the arguments. */
/* Retorna o deslocamento de uma var local em relacao a ix, dentro de
   uma funcao. Inclui os argumentos.  */
{                                             //              .      .
    int   lvsize = 0;                         //              |      | <- ix
    TPLST  locais = f->locais;                //            / |======|
    for (; locais; locais = locais->prox)  {  //          /   | vars |
        TITEM  *it = (TITEM*)locais->data;    // Function |   |locais|  /|\
        if (!strcmp(it->nome, nome))  break;  //          |   + -  - +   |
        lvsize += it->tipo->size;             //           \  | args |   addr's
    }                                         //            \ +------+   crescem
    return -(f->lsize - lvsize);              //                MEM
}


static TTIPO *get_tipo_var (char *nome, classe_item *cla, int *ldes)
/* Returns a variable's type, if it's local or global and, if it is local,
   return its ix stack offset too.If the variable doesn't exist, returns NULL.
   Returns if it is local/global in *cla.  */
/* Retorna o tipo de uma variavel, se ela e' local ou global, e no caso
 * de ser local retorna tb o seu deslocamento no stack ix. Se a var nao
 * existe, retorna NULL. Em *cla retorna se a var e' local ou global.   */
{
    TITEM  pr, *it;
    // Procurar a variavel nas vars locais   (look in the local variables)
    pr.nome = nome;
    it = proc_ele_lst(Func->locais, &pr, proc_item_fn);
    *cla = ciLOCAL;
    if (it)  {
       *ldes = get_lvar_des(Func, nome);   // Obter o desloc. da var   (get the variable's offset)
       return it->tipo;
    }
    *cla = ciGLOBAL;
    // Se nao foi encontrada, entao procurar nas vars globais   (if not found, look in the globals)
    it = proc_ele_lst(lst_gvars, &pr, proc_item_fn);
    // Retornar ponteiro para o tipo ou NULL se a var nao existe
    return (it? it->tipo : NULL);
}


static void add_desl (int desl)
/* Add a constant to hl. */
/* Gera codigo que adiciona uma konstante a hl */
{
     if (!desl)  return;
     emitPUSH(0, "de");
     emitLDint(0, "de", desl);
     emit(0,"%o%d%s", "add", "hl", "de");
     emitPOP(0, "de");
}


static int gera_toks (TPLST lv, TTIPO *t, TINSTR *ei)
/* Generates code to load hl with the offset to access a variable. Returns the variable's size. */
/* Gera codigo para colocar em hl o offset de um acesso a uma variavel. *
 * Retorna o tamanho da variavel.                                       */
{
    TITEM   *it;
    int     ofs, size, sele;
    TPLST   l;
    //print_vars(lv, 1);
    //print_tipo(t);printf("\n");
    for (; lv; lv = lv->prox)  {
        TVAR    *v = (TVAR*)lv->data;

        switch (v->tok)  {
           case vtSETA :
                DEB("\t;seta...\n");
                if (t->tipo == tPONT)  {
                    t = t->u.tipo;
                    if (t->tipo != tSTRU)
                        Erro(-11, ei, "Variable isn't a struct");
                }
                else Erro(-10, ei, "Variable isn't a pointer");
                emit_load_gptr();     // hl <- (hl)

           case vtSTRU :
                if (v->tok == vtSTRU)  DEB("\t;estrutura...\n");
               // Procurar o campo, enquanto se somam os tamanhos dos campos
               // anteriores, para se obter o offset do campo que se quer
                for (ofs = 0, l = t->u.campos; l; l = l->prox)  {
                    it = (TITEM*)l->data;
                    if (!strcmp(it->nome, v->u.nome))  break;
                    ofs += it->tipo->size;
                }
               // Erro se o campo nao foi encontrado
                if (strcmp(it->nome, v->u.nome))
                   Erro(-9, ei, "Unknown structure field");

                add_desl(ofs);
                t = it->tipo;
                break;

           case vtSUBS :
                emitPUSH(0, "de");  emitPUSH(0, "bc");  emitPUSH(0, "af");

                emitPUSH(0, "hl");       // Guardar o addr da var
                size = gera_expr(v->u.index, ei);
                emitPOP(0, "hl");        // Recuperar o valor da expressao
                if (size != 2)  {        // Cast para 16 bits unsigned
                   emitLDreg(0, "l", "h");
                   emitLDint(0, "h", 0);
                }

                t = t->u.array.tipo_elem;
                sele = t->size;
                if (sele == 1) ;
                else if (sele == 2)
                    emit(0,"%o%d%s", "add", "hl", "hl");                                        
                else  {
                    emitLDint(0, "de", sele);
                    // hl = hl * de = elem_size * index
                    emit(0,"%o%d", "calls", "_HLumulDE");
                    if (cmdl.tipo_prog == tpDLL)
                       lst_code->lbl = dupstr(getrlbl(getln()));
                    RoNu.mul16 = TRUE;
                }
                emit(0,"%o%d%s", "ex", "de", "hl");
                emitPOP(0, "hl");        // Recuperar o addr da var
                emit(0,"%o%d%s", "add", "hl", "de");

                emitPOP(0, "af");  emitPOP(0, "bc");  emitPOP(0, "de");
                break;
        }
    }

   // t fica com o tipo base. Retornar o 'tamanho da variavel'.
    return t->size;
}



gera_var (TPLST lv, TINSTR *ei)
/* Generate code to load hl with the address of a memory location. Return the variable's size. */
/* Gera codigo para colocar o endereco da posicao de mem em hl. *
 * Retorna o tamanho da variavel.                               */
{
    TPLST   vid = procura_tok(lv, vtVAR);   // Procurar vtID, o nome da variavel
    TVAR    *v;
    TTIPO   *t;
    int    ha_asts = 0, size, ldes;
    classe_item  classev;

   // Se e' uma string, trata ja' e sai  (if it's a string, process it and return)
    v = (TVAR*)lv->data;
    if (v->tok == vtSTRING)  {
       DEB("\t;string\n");
       emitLDreg(0,"hl", v->u.nome);
       return 0;
    }

   // Se e' uma chamada, trata ja' e sai    (if it's a call, proccess it and return)
    if (v->tok == vtCALL)  {
       size = gera_fn_call(&v->u.call, ei);
       // Can't let the function return void, otherwise it can "crash" the code which is going to receive size = 0.
       // Nao se pode deixar que a funcao retorne void, pois pode
       // 'baralhar' o codigo que vai receber 0 no size.
       if (!size)  Erro(-124, ei, "Called function returns void");
       return size;
    }

    if (v->tok == vtASTS)  ha_asts = 1;

    if (!v)  Erro(100, ei, "Invalid variable");
    v = (TVAR*)vid->data;

   // Obter o tipo da variavel   (get the variable's type)
    t = get_tipo_var(v->u.nome, &classev, &ldes);
    if (t == NULL)  ErroS(101, ei, "Variable '%s' does not exist", v->u.nome);

    if (classev == ciGLOBAL)
       emitLDreg(0,"hl", v->u.nome);        // Obter offset da var
    else  {
       emitPUSH(0,"ix");
       emitPOP(0,"hl");
       add_desl(ldes);
    }
    size = gera_toks(vid->prox, t, ei);

   // Agora tratar os asteriscos   (now process the asterisks)
    if (ha_asts)  {
       int   i;
       for (i = ((TVAR*)lv->data)->u.nasts; i; i--)  {
           if ((t->tipo != tPONT) && (t->tipo != tARRAY))
              ErroS(102, ei, "Variable '%s' ins't a pointer", v->u.nome);
           emit_load_gptr();     // hl <- (hl)
           t = t->u.tipo;
           size = t->size;  // O size e' o do tipo apontado (derreferenciado)
       }
    }

    return size;
}




static void gera_cast (TCODE **lst, int size_var, int size, int n_operan)
/***** Incomplete: still needs to verify the sign...
   size_var is the variable's size to which we want to cast to size */
/***** INCOMPLETO:: FALTA verificar o sinal... */
/* size_var e' o size da var a que se quer fazer o cast para size */
{
    if (size_var == size)
       return;
    if (size_var > size)  {
       emitLDreg(lst, RTab[0][n_operan], RTab1[n_operan][1]);
    } else {
       emitLDreg(lst, RTab1[n_operan][1], RTab[0][n_operan]);
       emitLDint(lst, RTab1[n_operan][0], 0);
    }
}


static int gera_operando (TEXPR *ex, int n_operan, TINSTR *ei)
/* Generates an operand. n_operand says if it's the 1st or 2nd operand of the expression.
   This information influences the CPU register where the operand is going to be place.
   If the operand is a constant, ld_konst is left pointing to the load operation, otherwise
   it returns with NULL. */
/* n_operan indica se este operando e' o primeiro (0) ou segundo (1)
 * operando da expressao. Esta informacao influencia o registo em que
 * o operando vai ser colocado.
 * Se o operando e' uma konst, ld_konst e' deixado a apontar para a
 * operacao de load. Caso contrario sai daqui com NULL. */
{
    int     size_var;

    switch (ex->oper)  {
       case eKONS : emitLDint(0, RTab[1][n_operan], ex->u.k);
                    // Para ja' consideramos todas as constantes de 16bits  (for now, consider all constants to be 16bits in size)
                    return 2;
       case eVAR  : size_var = gera_var(ex->u.var, ei);  // So' poe o addr em hl   (only puts the addr in hl)
                    // Se nao era uma chamada a uma funcao, carregar o valor    (if it was a call, load the variable,
                    // da variavel, c.c., HL ou A ja' teem o valor.              otherwise hl or a already have the value)
                    if (((TVAR*)(ex->u.var)->data)->tok == vtCALL)  {
                       // Carregar o valor em A ou HL no registo certo.     (load the value in a, hl or the right register)
                       // Podem ser geradas redundancias (ld a, a) mas      (redundancies may be generated but will be
                       // depois a optimizacao trata disso.                  removed in the optimization step)
                       if (size_var == 1)
                          emitLDreg(0, RTab[0][n_operan], "a");
                       else {
                          emitLDreg(0, RTab1[n_operan][1], "l");
                          emitLDreg(0, RTab1[n_operan][0], "h");
                       }
                    }
                    else
                       if (size_var == 1)
                           emitLDreg(0, RTab[0][n_operan], "(hl)");
                       else
                          if (RTab[1][n_operan][1] != 'l')  {
                             emitLDreg(0, RTab1[n_operan][1], "(hl)");
                             emitINC(0, "hl");
                             emitLDreg(0, RTab1[n_operan][0], "(hl)");
                          } else {
                             emitLDreg(0, "a", "(hl)");  // (posso usar 'a' pq a var       (register a is not in use
                             emitINC(0, "hl");           // e' 16 bit, logo 'a' nao         because variable is 16 bits)
                             emitLDreg(0, "h", "(hl)");  // esta' em uso)
                             emitLDreg(0, "l", "a");                        /**/
                          }
                    return size_var;
       case eADDR : gera_var(ex->u.var, ei);
                    return 2;
       case eSTRING :   // String (tem que) aparece sempre isolado    (a string always has to come up isolated)
                    emitLDreg(0, "hl", ex->u.string);
                    return 2;
       default :
            ErroI(-4, "gera_operando - unknown operand");
    }
}



static void emit_expr_bin (char *op, int size)
/* Generates code for 1 operation, considering it's size (8/16 bits) */
/* size e' o tamanho da expressao (= ao tamanho do 'maior' operando).
   Gera codigo p/ 1 operacao, tendo em conta o tamanho dela (8 ou 16 bits).
   STILL DOES NOT SUPPORT OPERAND SIGNAL. EVERYTHING UNSIGNED. ********/
{
    char    k;
    switch (size)  {
        case 1 : k = 1;
                 if (!strcmp(op,"mul"))
                    emit(0,"%o%d", "calls", "_AumulB"), RoNu.mul8 = TRUE;
                 else if (!strcmp(op,"div"))
                    emit(0,"%o%d", "calls", "_AudivB"), RoNu.div8 = TRUE;
                 else if (!strcmp(op,"mod"))
                    emit(0,"%o%d", "calls", "_AumodB"),  RoNu.mod8 = TRUE;
                 else if (!strcmp(op,"srl"))
                    emit(0,"%o%d", "calls", "_AshrB"), RoNu.shr8 = TRUE;
                 else if (!strcmp(op,"sla"))
                    emit(0,"%o%d", "calls", "_AshlB"), RoNu.shl8 = TRUE;
                 else  {
                    emit(0,"%o%s", op, RTab[0][1]); k = 0;
                 }
                 if ((cmdl.tipo_prog == tpDLL) && k)
                    lst_code->lbl = dupstr(getrlbl(getln()));
                 break;
        case 2 : k = 1;
                 if (!strcmp(op,"sub"))  {
                    emit(0,"%o%s", "or", "a");       // CF = 0
                    emit(0,"%c", "sbc", RTab[1][0], RTab[1][1]);
                    k = 0;
                  }
                 else if (!strcmp(op,"add"))  {
                    emit(0,"%c", op, RTab[1][0], RTab[1][1]);  k = 0;  }
                 else if (!strcmp(op,"mul"))
                    emit(0,"%o%d", "calls", "_HLumulDE"), RoNu.mul16 = TRUE;
                 else if (!strcmp(op,"div"))
                    emit(0,"%o%d", "calls", "_HLudivDE"), RoNu.div16 = TRUE;
                 else if (!strcmp(op,"mod"))
                    emit(0,"%o%d", "calls", "_HLumodDE"), RoNu.mod16 = TRUE;
                 else if (!strcmp(op,"srl"))
                    emit(0,"%o%d", "calls", "_HLshrDE"), RoNu.shr16 = TRUE;
                 else if (!strcmp(op,"sla"))
                    emit(0,"%o%d", "calls", "_HLshlDE"), RoNu.shl16 = TRUE;
                 else {
                     emitLDreg(0,"a", RTab1[0][1]);
                     emit(0,"%o%s", op, RTab1[1][1]);
                     emitLDreg(0,RTab1[0][1], "a");

                     emitLDreg(0,"a", RTab1[0][0]);
                     emit(0,"%o%s", op, RTab1[1][0]);
                     emitLDreg(0,RTab1[0][0], "a");
                     k = 0;
                 }
                 if ((cmdl.tipo_prog == tpDLL) && k)
                    lst_code->lbl = dupstr(getrlbl(getln()));
                 break;
        default : ErroI(-231, "; Unknown size!! (emit_expr_bin)");
    }
}


static void gera_oper_bin (TEXPR *ex, int size, TINSTR *ei)
/* 'size' is the expression's size (= size of the bigger operand).   */
{
    switch (ex->oper)  {
       case eMUL  : emit_expr_bin("mul", size);           break;
       case eDIV  : emit_expr_bin("div", size);           break;
       case eMOD  : emit_expr_bin("mod", size);           break;
       case eSHR  : if ((ex->u.bin.d->oper == eKONS) && (ex->u.bin.d->u.k ==1))
                       emit(0,"%o%d", "srl", "a");
                    else
                       emit_expr_bin("srl", size);
                    break;
       case eSHL  : if ((ex->u.bin.d->oper == eKONS) && (ex->u.bin.d->u.k ==1))
                       emit(0,"%o%d", "sla", "a");
                    else
                       emit_expr_bin("sla", size);
                    break;
       case eADD  : emit_expr_bin("add", size);           break;
       case eSUB  : emit_expr_bin("sub", size);           break;
       case eAND  : emit_expr_bin("and", size);           break;
       case eOR   : emit_expr_bin("or", size);            break;
       case eXOR  : emit_expr_bin("xor", size);           break;
       default :
            printf("%i\n", ex->oper);
            Erro(-4, ei, "Unknown operator (gera_oper_bin)");
    }
}


int reload_konst (TEXPR *ex, TCODE *i, int size_k, int size_outro, int n_operan,
                  int push_tambem, TINSTR *ei)
/* Fixes an LD of a constant value, changing its size.
   if (push_tambem != 0) then if a push follows the LD, it is changed also. */
/* Corrige um LD de uma konstante, alterando o seu tamanho.
   Se (push_tambem != 0) entao tambem e' alterado um push que se siga ao LD */
{
    if (ex->oper != eKONS) return size_k;
    if (size_k == size_outro) return size_k;
   // Tornar a konst com o mesmo size do outro lado da expressao    (make the constant's size the same as the other expression's side)
    if (size_outro == 1)
       if ( (atoi(i->src) > 255) || (atoi(i->src) < -128) )
          Erro(-7, ei, "Constant doesn't fit into 8 bits");
    if (size_outro == 2)
       if ( (atoi(i->src) > 65535) || (atoi(i->src) < -32768) )
       Erro(-9, ei, "Constant doesn't fit into 16 bits");
    free(i->dst);
    i->dst = meulloc(4);
    strcpy(i->dst, RTab[size_outro-1][n_operan]);
    if (push_tambem == PUSHNAO)
       return size_outro;   // O novo size da konst    (the new constant size)
    i = i->prox;
    free(i->src);
    i->src = meulloc(4);
    strcpy(i->src, RegPar(RTab[size_outro-1][n_operan]));
    return size_outro;      // O novo size da konst
}


static int get_size_expr (int s1, int s2)
{   return (s1 > s2)?  s1: s2;   }


static gera_expr_ (TEXPR *ex, TINSTR *ei)
{
    int     size_e, size_d, size, aux;
    TPCODE  co;

    if (ex->oper == eEXPR)
       return gera_expr_(ex->u.expr, ei);

    if E_OPER_BIN(ex->oper)  {
       switch ( (E_TERMINAL(ex->u.bin.e)) << 1 | (E_TERMINAL(ex->u.bin.d)) )  {
           case 0 :   // Nenhum e' terminal   (none is terminal)
                size_e = gera_expr_(ex->u.bin.e, ei);
                size_d = gera_expr_(ex->u.bin.d, ei);
                size = get_size_expr(size_e, size_d);
                switch (ex->oper)  {
                   case eDIV : case eSUB : case eMOD : case eSHL : case eSHR :
                        emitPOP_OPERAN(0, size, SEG);
                        gera_cast(0, size_d, size, SEG);
                        emitPOP_OPERAN(0, size, PRI);
                        gera_cast(0, size_e, size, PRI);
                        break;
                  // More efficient like this, save one instruction, but it swaps the operands
                  // leading to incorrect code for non comutative operations.
                  // Assim e' + eficiente, poupa 1 instr, mas troca os operandos
                  // pelo que geraria codigo incorrecto para operacoes nao comutativas
                   default :
                        emitPOP_OPERAN(0, size, PRI);
                        gera_cast(0, size_e, size, PRI);
                        emitPOP_OPERAN(0, size, SEG);
                        gera_cast(0, size_d, size, SEG);
                        break;
                }
                break;
           case 1 :   // d e' terminal    (d is terminal)
                size_e = gera_expr_(ex->u.bin.e, ei);
                size_d = gera_operando(ex->u.bin.d, SEG, ei);

                size_d = reload_konst(ex->u.bin.d, lst_code, size_d, size_e,
                                      SEG, PUSHNAO, ei);
                // Saves a reference to the reference's end, so that later in ca introduce a cast if necessary.
                // Guardar uma ref p/ o final da referencia, p/ + tarde
                // se poder introduzir um cast em caso de necessidade.
                co = lst_code;
                emitPOP_OPERAN(0, size_e, PRI);
                size = get_size_expr(size_e, size_d);
                gera_cast(&co, size_d, size, SEG);
                gera_cast(0, size_e, size, PRI);
                break;
           case 2 :   // e e' terminal
                size_d = gera_expr_(ex->u.bin.d, ei);
                size_e = gera_operando(ex->u.bin.e, PRI, ei);

                size_e = reload_konst(ex->u.bin.e, lst_code, size_e, size_d,
                                      PRI, PUSHNAO, ei);

                size = get_size_expr(size_e, size_d);
                gera_cast(0, size_e, size, PRI);
                emitPOP_OPERAN(0, size_d, SEG);
                gera_cast(0, size_d, size, SEG);
                break;
           case 3 :   // Ambos sao terminais    (both are terminal)
                size_d = gera_operando(ex->u.bin.d, SEG, ei);
                co = lst_code;
                size_e = gera_operando(ex->u.bin.e, PRI, ei);

                size_d = reload_konst(ex->u.bin.d, co, size_d, size_e, SEG, PUSHNAO, ei);
                size_e = reload_konst(ex->u.bin.e, lst_code, size_e, size_d, PRI,
                                      PUSHNAO, ei);

                size = get_size_expr(size_e, size_d);
                gera_cast(&co, size_d, size, SEG);
                // If the left side is a function call, then we need to save the right side in the stack,
                // which is in a register (RTab[0/1][1]) and recover it later after the call.
                // Se o lado esquerdo e' uma chamada a uma funcao, entao
                // e' preciso guardar no stack o valor do lado direito,
                // que se encontra num registo (RTab[0/1][1]) e recupera'-lo
                // depois da chamada.
                aux = (ex->u.bin.e->oper == eVAR) &&
                      ( (((TVAR*)((ex->u.bin.e)->u.var)->data)->tok) == vtCALL);
                if (aux)
                   emitPUSH(&co, RegPar(RTab[size-1][SEG]));
                gera_cast(0, size_e, size, PRI);
                if (aux)
                   emitPOP(0, RegPar(RTab[size-1][SEG]));
                break;
       }
       gera_oper_bin(ex, size, ei);
       emitPUSH_OPERAN(0, size);
    } else  {
       size = gera_operando(ex, PRI, ei);
    }
    return size;
}



int Pri (TEXPR *ex)
{
    switch (ex->oper)  {     // Prioridades da operacoes    (operator precedences)
        case eADD : case eSUB : return 0;    // - prioritario     (takes precedence)
        case eMUL : case eDIV : return 1;
        case eAND : case eOR  : case eXOR : return 2;
        case eSHL : case eSHR : return 3;
        case eEXPR : return Pri(ex->u.expr);
        default : return -1;
    }
}


TEXPR *dir2esq (TEXPR *ex)
{
    if E_OPER_BIN(ex->oper)  {
       TEXPR  *nex = ex->u.bin.d;
       ex->u.bin.d = nex->u.bin.e;
       nex->u.bin.e = ex;
       assert(nex->oper != eEXPR);   //*****//
       return nex;
    }
    else return ex;
}


TEXPR *MudaAssociatividadeParaEsq (TEXPR *ex, int pri)
{
    // Eliminar o 'token' parentesis   (remove the parentesis token)
    while (ex->oper == eEXPR)  {
       TEXPR  *ax = ex;
       ex = ex->u.expr;
       free(ax);    // deixa lixo ?     (does it leave any garbage?)
    }

    if E_TERMINAL(ex)
       return ex;

    if E_OPER_BIN(ex->u.bin.e->oper)
       ex->u.bin.e = MudaAssociatividadeParaEsq(ex->u.bin.e, Pri(ex->u.bin.e));

    // Percorrer a arvore para a direita, convertendo-a    (go through the tree left->right, converting it)
    while ((Pri(ex->u.bin.d) == pri) && E_OPER_BIN(ex->u.bin.d->oper))  {
       ex = dir2esq(ex);
       //ex = MudaAssociatividadeParaEsq(ex, pri);
       // Converter o ramo esquerdo (opers + prioritarias)   (convert the left side (hgher precedence operators))
       ex->u.bin.e = MudaAssociatividadeParaEsq(ex->u.bin.e, Pri(ex->u.bin.e));
    }
    // We either got to a higher precedence operator on the right side, or it's terminal.
    // Chegamos a uma oper + prioritaria no ramo direito, ou e' terminal
    ex->u.bin.d = MudaAssociatividadeParaEsq(ex->u.bin.d, Pri(ex->u.bin.d));
    return ex;
}



gera_expr (TEXPR *ex, TINSTR *ei)
/* After this function, the generated code puts the result in the stack.
   The function returns the expression's size. */
/* Apos esta funcao, o codigo gerado coloca o resultado na pilha.
   A funcao retorna o tamanho da expressao. */
{
    if E_TERMINAL(ex)  {
       int  size = gera_operando(ex, PRI, ei);
       if (ex->oper == eKONS)  {
          if (size == 2)  {
             free(lst_code->dst);
             lst_code->dst = dupstr("bc");
             emitPUSH(0,"bc");
          }
       }
       else
          emitPUSH_OPERAN(0, size);
       return size;
    }
    else  {
       ex = MudaAssociatividadeParaEsq(ex, Pri(ex));
       return gera_expr_(ex, ei);
    }
}


/** EXPRESSOES                                                             **/
/****************************************************************************/


gera_atribuicao (TATTR *a, TINSTR *ei)
{
    int    size_expr, size_var;
    TPLST  aux;
    TCODE  *ldk;

    DEB("\t;attr;  expr\n");
    check_size(size_expr = gera_expr(a->rvalue, ei), ei); // Poe o resultado na pilha   (puts the result in the stack)
    ldk = lst_code->ante;

    DEB("\t;var\n");
    aux = a->lvalue;
    check_size(size_var = gera_var(a->lvalue, ei), ei);   // Poe em hl o addr da var    (hl = variable's address)

    // Se a expr e' 1 konst, igualar o size da konst ao do lvalue   (if the expression is a constant, make constant size = lvalue)
    size_expr = reload_konst (a->rvalue, ldk, size_expr, size_var, PRI, PUSHTB, ei);

    // Gerar codigo para guardar o valor, fazendo os casts necessarios.   (gen code to save the value, doing the necessary casts)
    if (size_expr == 1)  {
       emitPOP(0, "af");
       emitLDreg(0,"(hl)", "a");        // Guardar o valor na var (8 bits)    (save the value in the variable (8 bits))
       if (size_var == 2)  {
          emitINC(0,"hl");
          emitLDint(0,"(hl)", 0);       // Por um 0 na parte alta  *** FALTA TRATAR SINAL   (puts 0 in the MSB ***** sign not being considered)
       }
    } else {
       emitPOP(0, "bc");                // Obter o valor da expressao   (get the expression's value)
       if (size_var == 1)
          emitLDreg(0,"(hl)", "c");
       else if (size_var == 2)  {
          emitLDreg(0,"(hl)", "c");        // Guardar a parte baixa do valor na var   (save the LSB in the variable)
          emitINC(0,"hl");
          emitLDreg(0,"(hl)", "b");
       }
    }
}





gera_condicao (TCOND *c, uint label_index, TINSTR *ei)
/* If the condition doesn't hold, label_index is to where it should jump next. */
// Se a condicao nao se verificar, label_index e' para onde deve saltar
{
    TCODE  *ldk;
    int    size_e, size_d, size_expr, sit;

    DEB("\t;cond\n");
    check_size(size_e = gera_expr(c->e, ei), ei);

    if (c->oper != orCBOOL)  {
       ldk = lst_code;
       check_size(size_d = gera_expr(c->d, ei), ei);

       if (c->d->oper == eKONS)  size_expr = size_e;
       else if (c->e->oper == eKONS)  size_expr = size_d;
       else  size_expr = get_size_expr(size_e, size_d);

       size_d = reload_konst(c->d, lst_code->ante, size_d, size_expr, PRI, PUSHTB, ei);
       size_e = reload_konst(c->e, ldk->ante, size_e, size_expr, SEG, PUSHTB, ei);

       emitPOP(0, RegPar(RTab[size_d-1][PRI]));
       gera_cast(&lst_code, size_d, size_expr, PRI);
       if (c->oper == orMAIOROUIGUAL)
          emit(0,"%o%d", "dec", RTab[size_expr-1][PRI]);
      // else if (c->oper == orMENOR)
      //    emit(0,"%o%d", "inc", RTab[size_expr-1][PRI]);
       emitPOP(0, RegPar(RTab[size_e-1][SEG]));
       gera_cast(&lst_code, size_e, size_expr, SEG);

       if (size_expr == 1)  emit(0,"%o%s", "cp", RTab[0][1]);
       else {
          emit(0,"%o%d", "or", "a");     // P/ fazer CF = 0 por causa do sbc
          emit(0,"%c", "sbc", "hl", "de");
       }
    }
    else
        emitPOP(0, RegPar(RTab[size_e-1][PRI]));

    // 1st operand in ??/? (depends on RTab) and 2nd in hl/a. If it's CBOOL, the operand is in hl/a.
    // O primeiro operando esta' em ??/? e o segundo em hl/a. Se e' CBOOL
    // o operando esta' em hl/a    (^^^^ depende de RTab)

    // e=2, d=1
    // (d - e) < 0   =>  (e > d)   =>  C        ->   NC
    // (d - e) > 0   =>  (e < d)   =>  NC e NZ  ->   C ou Z
    // (d - e) >= 0  =>  (e <= d)  =>  NC       ->   C
    // (d - e) <= 0  <=> (d - e -1) < 0   =>  (e >= d)  =>  C   ->   NC
    // old:
    // (d - e) > 0   <=> (d - e +1) >= 0  =>  (e < d)   =>  NC  ->   C
    // (d - e) <= 0  =>  (e >= d)  =>  C ou Z   ->   NC e NZ

    switch (c->oper)  {
       case orCBOOL :     if (size_e == 1) emit(0,"%o%s", "and", "a");
                          else {       emit(0,"%o%d%s", "ld", "a", "l");
                                       emit(0,"%o%s", "or", "h");
                          }
       case orDIFERENTE : emit(0,"%o%s%d", cmdl.tjmp, "z", getlbl(label_index));
                          break;
       case orIGUAL :     emit(0,"%o%s%d", cmdl.tjmp, "nz", getlbl(label_index));
                          break;
       case orMAIOROUIGUAL :
       case orMAIOR :     emit(0,"%o%s%d", cmdl.tjmp, "nc", getlbl(label_index));
                          break;
       case orMENOR :     emit(0,"%o%s%d", cmdl.tjmp, "c", getlbl(label_index));
                          emit(0,"%o%s%d", cmdl.tjmp, "z", getlbl(label_index));
                          break;
       case orMENOROUIGUAL :
                          emit(0,"%o%s%d", cmdl.tjmp, "c", getlbl(label_index));
                          break;
       default : ErroI(-35, "Unknown relational operator");
    }
}




gera_bloco (TPLST bloco, uint out_label_index)
/* out_label_index it the current block's exit label. */
/* out_label_index e' um label de saida do bloco actual. */
{
    static char *get_srcline_frmt(TFINFO *pos);
    uint  olabel;
    int   size_aux;

    for (; bloco; bloco = bloco->prox)  {
        TINSTR   *i = (TINSTR*)bloco->data;

        if (i == NULL)  continue;     // Instrucao vazia
        if ((i->itipo != iASM) && (i->itipo != iNULA))
            emit(0,"%t", get_srcline_frmt(&i->posi));

        switch (i->itipo)  {
           case iNULA :
                break;

           case iRETURN :
                DEB("\t;return\n");
                if (i->u.expr_ret)  {
                   if (Func->tipo_ret->size == 0)
                        WarningS(i, "void function returning value (%s)\n",
                                 i->posi.nome);
                   else  {
                      check_size(size_aux = gera_expr(i->u.expr_ret, i), i);
                      // If expr was 1 constant, convert the load to the return size.
                      // Se a expr era 1 konst, refazer o load p/ o size de retorno
                      size_aux = reload_konst(i->u.expr_ret, lst_code->ante, size_aux,
                                              Func->tipo_ret->size, PRI, PUSHTB, i);
                      // Get the value of the return expression.
                      // Obter o valor da expressao de retorno
                      if (size_aux == 1)   emitPOP(0,"af");
                      else                 emitPOP(0,"hl");
                      // Make the cast to the return value.
                      // Fazer o cast do valor da expr p/ o valor de retorno da func
                      gera_cast(0, size_aux, Func->tipo_ret->size, 0);
                   }
                }
                else if (Func->tipo_ret->size != 0)
                        WarningS(i, "function should return a value (%s)\n",
                                 i->posi.nome);
                // Saltar para o label de saida da funcao        (jump to the function's exit label)
                emit(0,"%o%s", cmdl.retjmp, getlbl((uint)Func));
                break;

           case iBREAK :
                DEB("\t;break\n");
                emit(0,"%o%s", cmdl.tjmp, getlbl(out_label_index));
                break;

           case iFOR :
                DEB("\t;for/while\n");
                olabel = (uint)i;
                if (i->u.ifor.aini != NULL)
                   gera_atribuicao(i->u.ifor.aini, i);
                emit(0,"%l", getlbl(olabel));
                if (i->u.ifor.cond != NULL)
                   gera_condicao(i->u.ifor.cond, olabel+2, i);
                gera_bloco(i->u.ifor.corpo, olabel+2);
                emit(0,"%t", "\t; - - -\n");
                if (i->u.ifor.aciclo != NULL)
                   gera_atribuicao(i->u.ifor.aciclo, i);
                emit(0,"%o%s", cmdl.tjmp, getlbl(olabel));
                emit(0,"%l", getlbl(olabel+2));
                break;

           case iSWITCH : {
                TCASELST   *cl;
                if (gera_expr(i->u.swi.valor, i) != 1)
                   Erro(-11, i, "'switch' value must be an 8bit expression");
                emitPOP(0, RegPar(RTab[0][PRI]));

                for (cl = i->u.swi.cases; cl; cl = cl->prox)  {
                    emit(0,"%o%is", "cp", cl->tval);
                    emit(0,"%o%s%d", "jr", "z", getlbl((uint)cl));
                }
                emit(0,"%o%s", "jp", getlbl(((uint)(i)) + 1));
                for (cl = i->u.swi.cases; cl; cl = cl->prox)  {
                    emit(0,"%l", getlbl((uint)cl));
                    gera_bloco(cl->bloco, (uint)i);
                }
                emit(0,"%l", getlbl(((uint)(i)) + 1));
                gera_bloco(i->u.swi.Default, (uint)i);
                emit(0,"%l", getlbl((uint)i));
                break; }

           case iASM : {
                char  st[128];
                if ( ((TINSTR*)(bloco->ante->data))->itipo != iASM )
                   sprintf(st, " ;%12s %7i:  User asm code...\n",
                               i->posi.nome, i->posi.nlin);
                emit(0,"%t", st);
                emit(0,"%t", i->u.iasm);
                emit(0,"%t", "\n");
                } break;

           case iIF :
           case iIFC : case iIFNC : case iIFZ : case iIFNZ :
                DEB("\t;if\n");
                switch (i->itipo)  {
                   case iIFC  : emit(0,"%o%s%d", "jr", "nc", getlbl((uint)(i))); break;
                   case iIFNC : emit(0,"%o%s%d", "jr", "c",  getlbl((uint)(i))); break;
                   case iIFZ  : emit(0,"%o%s%d", "jr", "nz", getlbl((uint)(i))); break;
                   case iIFNZ : emit(0,"%o%s%d", "jr", "z",  getlbl((uint)(i))); break;
                   case iIF : gera_condicao(i->u.iif.cond, (uint)(i), i); break;
                }

                gera_bloco(i->u.iif.corpo, out_label_index);
                if (i->u.iif.alter != NULL)  {
                   emit(0,"%o%s", cmdl.tjmp, getlbl((uint)(i) + 1));
                   emit(0,"%l", getlbl((uint)(i)));
                   gera_bloco(i->u.iif.alter, out_label_index);
                   emit(0,"%l", getlbl((uint)(i)+1));
                }
                else
                   emit(0,"%l", getlbl((uint)i));
                break;

           case iATTR :
                gera_atribuicao(i->u.iattr, i);
                break;

           case iCALL :
                gera_fn_call(&i->u.call, i);
                break;

           case iGETE :
                gera_var(i->u.var, i);
                break;

           case iGETV :
                if (gera_var(i->u.var, i) == 1)
                   emit(0, "%o%d%s", "ld", "a", "(hl)");
                else
                   emit_load_gptr ();
                break;

           default :
                ErroI(-23, "Unknown tree instruction...");
        }
    }
}




/***************************************************************************/

static int calc_size_local_data (TPLST ll)
/* Return the totl size (in bytes) of the local data (arguments + variables). */
/* Retorna o tamanho total (em bytes) dos dados locais (argumentos + vars) */
{
    int   lvsize = 0;
    for (; ll; ll = ll->prox)  {
        TITEM  *it = (TITEM*)ll->data;
        lvsize += it->tipo->size;
    }
    return lvsize;
}


gera_funcoes (ttprog tipo_prog, char *tjmp)
{
    TPLST   l = lst_funcs;

    for (; l; l = l->prox)  {
       // Func is a global variable, so that it can be accessed from the entire module.
       // Func e' uma variavel global, de modo a ser acessivel pelo resto do modulo.
        Func = (TFUNCAO*)(l->data);
       // If the function is external (no body), it doesn't get generated.
       // Se a funcao e' externa (nao tem corpo) nao se gera
        if (Func->corpo == NULL)
           continue;
       // If the function is not used, it isnt' generated.
       // S'a func nao e' usada, nao a gerar
        if (!fnOUTPUT(*Func))
           continue;
        emit(0,"%t", ";***********************************************************");
       // Exported functions have the string ";;EXPORT" after the label.
       // As funcoes exportadas teem a seguir ao label a stirng ";;EXPORT"
        if (fnEXPORT(*Func))   emit(0,"%l%t", Func->nome, "   ;;EXPORT\n");
        else                   emit(0,"%l", Func->nome);
       // Calcular o tamanho dos dados locais    (calculate local data size)
        Func->lsize = calc_size_local_data(Func->locais);
        if (Func->lsize > 128)
           ErroS(-20, 0, "Local data too big (128b max) (%s)", Func->nome);

       // Gerar um 'stack frame' (se houver dados locais)     (generate a stack frame, if there's any local data)
        if (Func->locais)
           switch (Func->lsize)  {
             case 2 : emit(0,"%o%d", "inc", "ix");
             case 1 : emit(0,"%o%d", "inc", "ix"); break;
             default : emit(0,"%o%d%is", "ld", "de", Func->lsize);
                       emit(0,"%o%d%s", "add", "ix", "de");
           }
        emit(0,"%t", "\n");
       // Gerar o codigo da funcao    (generate the function's code)
        gera_bloco(Func->corpo, (uint)Func);
       // Generate the function's exit label. There are 2: the 1st is the "normal" and the 2nd is to help inside an asm statment.
       // Label de saida da funcao. Sao dois: O primeiro e' o 'normal' e o
       // segundo e' para ajudar dentro dum asm {..}
        emit(0,"%l", getlbl((int)Func));//        emit(0,"%l", get_func_exit_lbl());
       // Restaurar o 'stack frame'    (restore the is stack frame)
        if (Func->locais)
           switch (Func->lsize)  {
             case 2 : emit(0,"%o%d", "dec", "ix");
             case 1 : emit(0,"%o%d", "dec", "ix"); break;
             default : emit(0,"%o%d%is", "ld", "de", -Func->lsize);
                       emit(0,"%o%d%s", "add", "ix", "de");
           }

        emit(0,"%o%t", "ret", "\n\n");
    }

    lst_code = ini_lst(lst_code);
}


/***************************************************************************/

static  FILE *fsrc;

static int open_source (char *fname)
{
    fsrc = fopen(fname, "rt");
    if (!fsrc)
       return -1;
    return 0;
}

static char *get_srcline_frmt (TFINFO *pos)
{
    static  char buf[8192];
    int  linha, u, len;

    if (open_source(pos->nome))
       ErroS(-123, 0, "(get_srcline_frmt) ** File not found (?) (%s).", pos->nome);
    sprintf(buf,"  %12s %7i: ", pos->nome, pos->nlin);
    len = strlen(buf);
    for (linha = 0; linha != pos->nlin; linha++)  {
        buf[len] = 0;
        fgets(buf+len, 8192-len, fsrc);
    }

    while (strchr(buf, '\n') == NULL)  {
        u = strlen(buf) - 2;
        fgets(buf+u+1, 8192-len-u, fsrc);
        linha++;
    }

    fclose(fsrc);
    buf[1] = ';';     // (So' agora q e' p/ nao confundir o while)
    return buf;
}



void gera_codigo_Z80 (TROTNUM *rotnum)
{
    lst_code = New(TCODE);
    memset(lst_code, 0, sizeof(TCODE));

    memset(&RoNu, 0, sizeof(RoNu));   // Assumir que nenhuma rotina e' usada      (assume no routine is used)


    gera_funcoes(cmdl.tipo_prog, cmdl.tjmp);

    *rotnum = RoNu;
}






