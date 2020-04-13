
#ifndef __Tipos_h_
#define __Tipos_h_


#include "ticlex.h"
#include "..\tipos\listas.h"


#define TRUE     -1
#define FALSE    0



void *meulloc (int);
void ErroF (int, char*);
void ErroI (int, char*);
char *dupstr (char*);
int cmpstr (char *, char *);



// What aritmetics asm routines should be included in the final code.
// TROTNUM contem vars bools que indicam se sao utilizadas algumas
// rotinas aritmeticas, de modo a que o writer de codigo so' coloque
// no file de saida as rotinas usadas.
typedef char  bool;
typedef struct {
   bool    mul8;
   bool    mul16;
   bool    div8;
   bool    div16;
   bool    mod8;
   bool    mod16;
   bool    shr8;
   bool    shl8;
   bool    shr16;
   bool    shl16;
 } TROTNUM;


enum ttprog { tpAPP, tpOVERLAY, tpDLL };
typedef enum ttprog ttprog;

// Opcoes, tiradas da linha de comandos   (command line options)
typedef struct {
   char    src_file[512];  // Nome do file C      (C file name)
   char    include_dir[512];   // Default directory for include files    (default include files directory)
   char    optimizar;      // Nivel de optimizacao [0..2]      (optimization level)
   ttprog  tipo_prog;      // Tipo de programa a compilar      (type of program to compile)
   bool    print_stats;    // Imprimir estatisticas            (print statistics?)
   bool    print_optim_stats;   // Imprimir stats da optimizacao (DEBUG)     (print optimization stats? debug)
   bool    reduce_init;    // Code 'init reduzido (so' 1 jp para main)       (reduce initialization code?)
   bool    simple_prgm;    // Compilar 1 prog simples: s/ stack nem jp p/ _main    (compile a simple program, no stack, jp nor jump to main)
   bool    estimar_tjmp;   // Deixar o TISCO escolher +- o tipo de jmp a usar     (let TISCO choose the jump size)
   char    tjmp[4];        // Tipo de jumps a usar: jp ou jr.      (what jump size to use, jp or jr)
   char    retjmp[4];      // Tipo de jump a usar em rets          (type of jump to use in call returns)
   bool    gen_args_code;  // Usar o parametro da main?            (use main's argument?)
 } TOPT;


// Estatisticas
typedef struct {
   int   mem_total_alocada;
 } TSTATS;




typedef unsigned int  uint;

typedef struct TEXPR  *TPEXPR;
typedef struct TTIPO  *TPTIPO;



/**************************************************************************/
/** EXPRESSOES   Expressions                                             **/

/* Indica se a operacao e' binaria */      // binary operation?
#define E_OPER_BIN(op__)       ( (op__ >= eADD) && (op__ <= eSHL) )
/* Indica se a operacao e' binaria e nenhum dos operandos e' uma expressao */   // binary op without expression operands?
#define E_OPER_TERMINAL(ex__)  ( E_OPER_BIN(ex__->oper) &&              \
                                 (!E_OPER_BIN(ex__->u.bin.e->oper)) &&  \
                                 (!E_OPER_BIN(ex__->u.bin.d->oper)) )
#define E_TERMINAL(ex__)       ( (!E_OPER_BIN(ex__->oper)) && (ex__->oper != eEXPR) )
/* Indica se uma operacao e' ou nao comutativa */    // comutative operation?
#define OPER_COMUTATIVA(op_)   ( ((op_) != eDIV) && ((op_) != eSUB)  &&  \
                                 ((op_) != eMOD) && ((op_) != eSHL)  &&  \
                                 ((op_) != eSHR) )

enum texpr  { eADD = 0, eSUB = 1, eMUL = 2, eDIV = 3, eAND = 4, eOR = 5,
              eXOR = 6, eSHR = 7, eMOD = 8, eSHL = 19, eEXPR = 20,
              eKONS = 22, eVAR = 23, eADDR = 24, eSTRING = 25,
//              ePLUSE = 26, eMINUSE = 27,      // ++ e +=k, -- e -=k
            };  // Ao acrescentar operacoes nao esquecer de actualizar as macros    (don't forget to update above macros when adding new oeprations)
                // anteriores e todos os switchs com operacoes...

typedef enum texpr texpr;


typedef struct TOPBIN {
    TPEXPR   e;
    TPEXPR   d;
  } TOPBIN;


typedef struct TEXPR {
    texpr  oper;
    union {
      char      *string;
      TOPBIN    bin;
      TPEXPR    expr;
      TPLST     var;      // Variavel (lista de TVAR)
      int       k;
    } u;
  } TEXPR;



/**************************************************************************/
/** TIPOS                                                                **/

enum ttipo  { tVOID = 0, tCHAR = 1, tUCHAR = 2, tINT = 3, tUINT = 4,
              tSTRU = 5, tPONT = 6, tARRAY = 7, tID = 8 };

typedef enum ttipo ttipo;


typedef struct TARRAY {
    int      nelem;
    TPTIPO   tipo_elem;
  } TARRAY;


typedef struct TTIPO {
    int      size;          // Tamanho do tipo em bytes
    ttipo    tipo;
    union {
      TPTIPO     tipo;        // Tipo 'apontado'
      TARRAY     array;
      TPLST      campos;      // Lista de TTIPO
      char       *nome;       // tID (nome do tipo qdo e' definido pelo user)
    } u;
  } TTIPO;



/**************************************************************************/
/** VARIAVEIS    Variables                                               **/

enum vartok  { vtVAR = 0, vtASTS = 1, vtSETA = 2, vtSUBS = 3, vtSTRU = 4,
               vtCALL = 5, vtSTRING = 6 };

typedef enum vartok vartok;


typedef struct TCALL  {
    char   *nome_fn;
    TPLST  args;           // Lista de variaveis (que sao listas de vartok's)   (variables list)
} TCALL;


typedef struct TVAR  {
    vartok  tok;
    union {
      char     *nome;      // vtVAR | vtSTRU | vtSETA
      TPEXPR   index;      // vtSUBS
      TCALL    call;
      int      nasts;      // nr de asteriscos
    } u;
  } TVAR;



/**************************************************************************/
/** CONDICOES                                                            **/

enum oper_rel  { orIGUAL = 0, orDIFERENTE = 1, orMAIOR = 2, orMENOR = 3,
                 orMAIOROUIGUAL = 4, orMENOROUIGUAL = 5, orCBOOL = 6 };

typedef enum oper_rel oper_rel;


typedef struct TCOND  {
    oper_rel    oper;    // Operador relacional    (relacional operator)
    TEXPR       *e;      // Expr do lado esquerdo  (left side operand)
    TEXPR       *d;      // Expr do lado direito (pode nao haver se cond = var)    (right side operand, may not exist)
  } TCOND;



/**************************************************************************/
/** ITEMS                                                                **/

/* Items sao variaveis num sentido mais geral da palavra. Cada *
   item tem uma classe.                                        */

enum classe_item  { ciGLOBAL = 0, ciLOCAL = 1, ciARG = 2, ciCAMPO = 3,
                    ciTIPO = 4,
                    ciGLBEXTRN = 5 };        // (Variavel global externa)

typedef enum classe_item classe_item;


typedef struct TITEM  {
    char           *nome;
    TTIPO          *tipo;
    classe_item    classe;   // Classe do item (var global/local/arg, tipo, ..)
  } TITEM;



/**************************************************************************/
/** INSTRUCOES                                                           **/

enum instr  { iNULA = 0, iATTR = 1, iCALL = 2, iIF = 3, iIFC = 4, iIFNC = 5,
              iIFZ = 6,  iIFNZ = 7, iFOR = 12, iRETURN = 13, iBREAK = 14,
              iASM = 15, iGETE = 16, iGETV = 17, iSWITCH = 18 };

typedef enum instr instr;


typedef struct TATTR  {
    TPLST   lvalue;        // Variavel
    TEXPR   *rvalue;       // Expressao a atribuir    (expression to assign to the variable)
  } TATTR;

typedef struct TIF  {
    TCOND   *cond;
    TPLST   corpo;
    TPLST   alter;
  } TIF;

typedef struct TFOR  {
    TATTR   *aini;       // NULL se nao ha'   (may be NULL)
    TCOND   *cond;
    TATTR   *aciclo;     // NULL se nao ha'
    TPLST   corpo;
  } TFOR;

typedef struct TCASELST {
    struct TCASELST   *ante;
    struct TCASELST   *prox;
    int      tval;      // valor a testar    (value to test)
    TPLST    bloco;     // bloco
  } TCASELST;

typedef struct TSWITCH {
    TEXPR     *valor;
    TCASELST  *cases;    // Lista de blocos de instrs    (instructions block list)
    TPLST     Default;   // Codigo para o default        (code for the default)
  } TSWITCH;


typedef struct TINSTR  {
    instr    itipo;
    TFINFO   posi;       // Info sobre a posicao da instrucao (file, linha)   (statment file position info)
    union {
      TCALL    call;
      TEXPR    *expr_ret;     // Expressao de retorno (return)     (return expression)
      TSWITCH  swi;
      TATTR    *iattr;
      TIF      iif;
      TFOR     ifor;
      TPLST    var;           // Para o iGETx
      char     *iasm;         // codigo asm    (assembly code)
      TPLST    lvars;         // Lista de vars locais (TITEM's)
    } u;
  } TINSTR;



/**************************************************************************/
/** FUNCOES                                                              **/

enum fnflags  { ffOUTPUT = 1, ffFORCE = 2, ffEXPORT = 4 };

typedef enum fnflags fnflags;

#define fnFORCE(fn)       ( ((fn).flags & ffFORCE) == ffFORCE )
#define SETfnFORCE(fn)    ((fn).flags |= ffFORCE)
#define fnOUTPUT(fn)      ( ((fn).flags & ffOUTPUT) == ffOUTPUT )
#define SETfnOUTPUT(fn)   ((fn).flags |= ffOUTPUT)
#define fnEXPORT(fn)      ( ((fn).flags & ffEXPORT) == ffEXPORT )
#define SETfnEXPORT(fn)   ((fn).flags |= ffEXPORT)


// Function
typedef struct TFUNCAO  {
    char     *nome;
    TTIPO    *tipo_ret;    // Tipo de retorno da funcao  (return type)
    TPLST    locais;       // Var locais e argumentos    (local variables and arguments)
    int      lsize;        // Tamanho, em bytes, das vars locais (args+lvar)   (total size of local+arguments data)
    TPLST    corpo;        // Corpo da funcao (lista de TINSR)     (function body)
    fnflags  flags;        // Flags respeitantes 'as funcoes     (flags)
    int      index;        // Index da funcao quando e' exportada numa DLL    (function index when a DLL being compiled)
  } TFUNCAO;


/**************************************************************************/
/** DLLS                                                                 **/

typedef struct TDLL *TPDLL;

typedef struct TDLL  {
    TPDLL     ante;     // back
    TPDLL     prox;     // next
    char      *nome;    // name
    TLST      *protos;  // prototypes
  } TDLL;



/**************************************************************************/
/** CODIGO GERADO   Generated Code  heavily used during optimization     **/
/** Esta estrutura de dados contem o codigo gerado. E' usado amplamente  **/
/** na optimizacao.                                                      **/

typedef struct TCODE *TPCODE;

typedef struct TCODE {
    TPCODE   ante;
    TPCODE   prox;
    char     *lbl;         // Possivel label (ou NULL)
    char     *op;          // Opcode (em ascii) ou NULL (se linha for coment)
    char     *dst;         // Arg destino (em ascii) ou NULL
    char     *src;         // Arg fonte (em ascii) ou NULL
    char     *txt;         // Texto extra no final da instr (coments) ou NULL
//    TPCODE   alvo;         // Qdo a instr implica salto, tem o destino.
//    int      ocup;         // Ocupacao dos registos. Serve na optimizacao
  } TCODE;



#endif


