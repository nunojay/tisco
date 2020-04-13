
 /* NOTE: Possible bug:
    when we permit labels in 'Optimiza', we are ignoring the user
    asm, which can bring problems if the optimizer optimizes looking
    to the instruction before the user's asm code and after the
    user's asm code. */

 /* NOTA: POSSIVEL BUG:
    quando se permitem labels em Optimiza, esta-se a ignorar o user asm,
    que pode trazer problemas se o optimizador optimiza olhando para
    a instr antes do user asm e para a instr depois do user asm.
    ********************************************   */


  /* ASM code optimization */

  /* Part of the optimization is made based in z80 instr pattern search,
     which can be modified or replaced by other sequences, smaller or
     faster. The code generator (geraz80.c) gens lots of redundant instrs,
     because it is much easier to do it like this, and remove the
     redundancy later.
     There are some global optimization functions, that look at the code
     from a global point of view.

     'Optimizar' is the function that executes all pattern search
     and replace, and takes 3 data structures:

       1. A TCODE list
       2. An array with a description of the instrs sequence to search.
          Each array element describes an instruction. Several instr
          opcodes can be given (space separated). The opcode's arguments
          can be defined in a rigid way (like this HL register) or in
          a "loose" or wildcard way (like saying 'any 8bit register').
          Normally an instruction described in this array is meant to
          appear in the code, but we can say the other way around,
          like "this instruction should not appear".
       3. A string with a small program in a small language designed
          to operate changes in code patterns, once they are found.
          This language is called "Optimization Micro Code Language" (OMC).

     When 'Optimizar' finds an instrs block that matches the given
     description (2nd parameter of Optimizar'), the OMC program is
     executed (interpreted :)) having the matched block as its target.
     The elements of the TCODE list that are not asm instructions (like
     comments) are ignored.
     Next I describe the OMC instructions. The OMC instrs are always
     capitalized (first letter); 'n' is a digit, '0' to '9'; 'o' is
     "D" for "destination" or "S" for "source"; 's' is a char sequence
     ended by '.'.

          * "Pn"
            Position (locate) the 'instructions cursor' (IC) at n ('0' to '9').
            There is an internal position cursor, that always points
            to an asm instruction inside the matched block. Lots of OMC
            instructions operate over "the current asm instr" (CAI), which is the
            asm instructions currently pointed by the IC. The first instr
            of the block is at position 0.
          * "A"
            Erase the current instruction.
          * "I"
            Insert a new instruction, empty, at IC.
          * "Os"
            Change the CAI's opcode.
          * "Ss"
            Change the CAI's source operand.
          * "Ds"
            Change the CAI's destination operand.
          * "Conon"
            Copies an operand (first 'on') to another operand (2nd 'on').
            Per example, "CS1D3" copies instr 1 source to instr 3 destination.
          * "Mo"
            Makes the CAI's 'o' a memory reference, putting it around '(' and ')'.
            (if CAI = "ld a, hl" then MS will make CAI = "ld a, (hl)")
          * "To"
            Makes the reverse of "Mo", removing the 1st and last chars of
            an operand.
          * "Jos"
            Appends (joins) 's' to the end of CAIS's indicated operand.
          * "Xo"
            Pre-appends the string "ix" to CAI's indicated operand.
          * "Eoon"
            Appends an operand (last 'on' to the end of a CAI's operand
            (1st 'o') (used for constants)
          * "Vonon"
            Checks if 2 operand are equal. If they aren't, the OMC ends.
          * ' '
            All spaces are ignored. Allows us to space instructions making
            the program more readable.

     About all TCODE's list elements when arriving this module, it is assumed:

        * All consts that appear isolated have explicit sign. This helps
          in identifying a string as a number and eases their concatenation
          with "ix" when necessary. Also eases adding constants, because
          we only have to concat the strings. TASM will do the rest.
        * All data (vars) labels and functions start by an underscore.
          All string's labels start by a capital "S". This eases
          the optimizing task and linkage with other external modules
          (like TI's libraries).
   */


  /* A optimizacao e' feita com base na procura de sequencias de
     instrucoes Z80, geradas pelo gerador de codigo, que podem ser
     substituidas por outras sequencias mais pequenas ou mais
     rapidas. O gerador de codigo gera muita instrucao redundante,
     pois e' mais facil gerar o codigo desta maneira, pois basta
     'olhar' muito localmente no codigo para o gerar, o que torna
     as coisas muito mais simples.
     O codigo gerado inicialmente aparece entao com bastantes pequenos
     blocos 'tipicos' de instrucoes, que podem ser optimizados
     (substituidos). Este tipo de optimizacao (por substituicao) e'
     assim muito dependente do gerador de codigo. Nao e' indicado
     para optimizar qualquer codigo.
     Este e' o chamado 'pephole optimizer'.

     Para proceder 'a pesquisa e substituicao de blocos de instrucoes
     foi criada uma funcao (Optimizar) que aceita tres estruturas de
     dados:

        * A lista com os TCODEs
        * Um array com uma descricao mais ou menos especifica do
          bloco de instrucoes a procurar. Cada posicao do array
          contem a descricao de uma instrucao, que devera' aparecer
          no bloco na mesma posicao em que aparece no array.
          O opcode da instrucao pode ser um entre n opcodes dados.
          Os seus argumentos podem ser definidos univocamente ou
          como argumentos mais genericos ('registo', em vez de 'a',
          ou 'referencia 'a memoria' em vez de '(hl)').
          Normalmente cada instrucao que aparece no array e' para
          aparecer no bloco, mas ha' a possibilidade de indicar o
          contrario: que a instrucao nao deve aparecer na posicao
          indicada. Isto pode ser usado se, por exemplo, queremos
          substituir "ld  hl, k; ld  a, (hl)" por "ld  a, (k)".
          Ora, nao podemos fazer esta substituicao se a seguir vem
          "ld  b, h".
        * Uma string com um programa numa pequena linguagem que
          permite proceder a alteracoes num bloco de instrucoes.
          E' designado por 'Programa de Micro Codigo de Optimizacao'.

     Quando e' encontrado um bloco de instrucoes que corresponde 'a
     descricao (dada pelo segundo parametro), o programa dado no
     terceiro parametro e' executado sobre o bloco de instrucoes.
     Os elementos da lista de TCODE que nao sao instrucoes assembly
     sao ignorados.
     As instrucoes de MCO disponiveis sao descritas de seguida. Na
     descricao, as instrucoes (MCO) sao sempre em maiusculas (
     primeira letra), 'n' e' um algarismo ('0' a '9'), 'o' e' "D"
     ou "S", que representa 'destino' ou 'fonte', 's' e' um conjunto
     de caracteres terminados por '.'.

          * "Pn"
            Posicionar o 'cursor de instrucoes' na pos n ('0' a '9').
            Existe um cursor interno de posicao, que aponta sempre
            para uma instrucao dentro do bloco. Muitas instrucoes
            operam sobre a 'instrucao actual', que e' a instrucao
            apontada pelo cursor de instrucoes (CI). As posicoes
            comemcam a contar-se na primeira instrucao do bloco (
            posicao 0).
          * "A"
            Apaga a instrucao actual.
          * "I"
            Insere uma nova instrucao, vazia, na posicao do CI.
          * "Os"
            Altera o opcode da instrucao actual.
          * "Ss"
            Altera o operando fonte da instrucao actual.
          * "Ds"
            Altera o operando destino da instrucao actual.
          * "Conon"
            Copia um operando indicado no primeiro 'on' para o
            segundo 'on'. Por exemplo "CS1D3" copia a fonte da instr
            um para o destino da instr 3.
          * "Mo"
            Transforma 'o' da instr actual numa referencia 'a memoria
            (envolvendo-a com '(' e ')').
          * "To"
            Tira o 1§ e ult chars de dst/src. A intencao e' obter o
            endereco de uma refecrencia 'a mem, transformar o operando
            numa "nao-referencia-'a-mem".
          * "Jos"
            Junta s ao final do operando indicado.
          * "Xo"
            Concatena "ix" ao inicio do operando indicado na instr
            actual.
          * "Eoon"
            Concatena um operando (ultimo 'on') ao fim do operando (
            primeiro 'o') da instr actual (usado p/ konsts).
          * "Vonon"
            Verifica se 2 operandos sao iguais. Se nao forem, o
            programa de MCO termina de imediato.
          * ' '
            Todos os espacos sao ignorados. Permite espacar instrucoes
            para melhor legibilidade (mas penaliza um pouco o speed,
            pois perde-se tempo a 'saltar' por cima deles).

  */

  /* Pressupostos sobre os elementos da lista de TCODE que chega aqui:

        * Todas as constantes que aparecem isoladas sao antecedidas
          pelo caracter de sinal. Isto ajuda na identificacao de um
          item como sendo um numero, e facilita a concatenacao com
          ix, quando necessario, para formar um acesso 'a memoria.
          Tambem ajuda a somar konsts, pois basta concatenar as strings.
        * Todos os labels de dados (vars) e funcoes sao precedidos por
          '_'. As strings teem labels iniciados em 'S'.
          Ajuda na identificacao como label e alem do mais os includes
          que ha' temos nomes das vars precedidas por '_' (como e'
          tipico do C).

  */


#include <ctype.h>
#include <string.h>
#include "listas.h"
#include "tipos.h"


extern TOPT    cmdl;          // Opcoes da linha de comandos
extern TCODE   *lst_code;     // Lista com o codigo gerado



void printcode (TCODE*);


// Frees a TCODE's memory. First the content, then the TCODE himself.
#define Del(e_)   {  free((e_)->lbl);  free((e_)->op);   free((e_)->src);  \
                     free((e_)->op);   free((e_)->dst);                    \
                     (e_) = edel_ele_lst(e_);  }


#define STRCMP(a_, b_)    (cmpstr(a_, b_) == 0)

#define LBL(c_)      (c_)->lbl
#define OP(c_)       (c_)->op
#define SRC(c_)      (c_)->src
#define DST(c_)      (c_)->dst

// Check if a string is a func, global var or string label.
// All global vars a funcs labels start by "_", strings start by "S".
#define LABEL(s_)    ( (s_) && ((*s_ == '_') || (*s_ == 'S')) )
// Check if a string is a numeric const.
// All numbers (consts) start with a digit or sign
#define KONST(s_)    ( (s_) && \
                       ((isdigit(s_[0])) || (s_[0] == '-') || (s_[0] == '+')) )
// Check if a string is a memory reference.
// Refs mem start by '('
#define MREF(s_)     ( (s_) && (*s_ == '(') )
// If a string is none of the above, it is a register.
#define REG(s_)      ( !LABEL(s_) && !KONST(s_) && !MREF(s_) )




void *meulloc(int);


char *strst (char *a, char *b)
{
    if (b)
       return strstr(a, b);
    else
       return NULL;
}




/***************************************************************************/
/** GLOBAL OPTIMIZATION  (Global optimizations)                           **/

int OG_PushPopReg (TCODE *lc, char *reg, int ign_call)
/* Remove push/pop pairs of a register when the register initialy had
   no value to save. */
{
    int    nivel, em_uso = 0, n = 0;
    TCODE  *au;

    for (; lc && lc->prox; lc = lc->prox)  {

        if (!OP(lc))  {
           em_uso = 0;
           continue;
        }
        if (STRCMP(OP(lc), "ret"))  {
           em_uso = 0;
           continue;
        }

        if (STRCMP(OP(lc), "push") && (strst(reg, SRC(lc))) && (!em_uso))  {
           TCODE   *la = lc->prox;
           // Remove the push/pop pair, if they use the same register
           nivel = 1;
           for (; la; la = la->prox)
               if STRCMP(OP(la), "pop")  {
                   nivel--;
                   if (STRCMP(DST(la), reg) && (nivel == 0)) {
                      Del(la);    // del pop
                      Del(lc);    // del push
                      n++;
                      break;
                   }
               }
               else if STRCMP(OP(la), "push")
                   nivel++;
               else if (OP(la) == NULL)  //(LBL(la) != NULL)
                   break;   // the pair must belong to the same C function
           }
        else  {
           if ( (SRC(lc)) && strst(reg, SRC(lc)) )  // Os 2 if's nao sao mutuamente exclusivos.
              em_uso = 0;               // dst always wins.
           if ((DST(lc)) && strst(reg, DST(lc)))
              em_uso = 1;
           if (STRCMP(OP(lc), "call") && !ign_call)
              em_uso = 1;
           if (STRCMP(OP(lc), "jp") || STRCMP(OP(lc), "jr"))
              em_uso = 0;
        }
    }
    return n;
}



int OG_PushPopRegComValor (TCODE *lc, char *reg)
/* Removes a push/pop pair when the register isn't changed between the pair */
{
    int   nivel, em_uso = 0, n = 0;

    for (; lc && lc->prox; lc = lc->prox)  {

        if (!OP(lc))  {
           em_uso = 0;
           continue;
        }

        if (STRCMP(OP(lc), "push") && STRCMP(SRC(lc), reg))  {
           TCODE   *la = lc->prox;
           // Remove the pair, if the pop is of the same register and the register
           // (or part of it) was not used inside the pair.
           nivel = 1;
           for (; la; la = la->prox)
               if STRCMP(OP(la), "pop")  {
                   nivel--;
                   if (STRCMP(DST(la), reg) && (nivel == 0))  {
                      Del(la);    // del pop
                      Del(lc);    // del push
                      n++;
                      break;
                   }
               }
               else if STRCMP(OP(la), "push")
                   nivel++;
               else if (OP(la) == NULL)  //(LBL(la) != NULL)
                   break;   // the pair must belong to the same C function
               else if (strst(reg, SRC(la)) || strst(reg, DST(la)))
                   break;  // If the register is used inside the pair, don't remove the pop
               else if (STRCMP(OP(la), "call"))
                   break;    // A call can change the register
        }
    }
    return n;
}


int OG_MoveLoadRegisto (TCODE *lc, char *reg)
/* Sometimes a register is loaded and pushed, only beeing used much later,
   sometimes even beeing poped to another register.
   This rourinte seeks such cases and moves the load+push to where
   it is used. Then, the pattern optimization will take care of it.
   It only works on HL.
 */
#define EPUSH(l_)     ( STRCMP(OP(l_), "push") && STRCMP(SRC(l_), reg) )
#define ELDK16(l_)    ( STRCMP(OP(l_), "ld") && STRCMP(DST(l_), reg) )//&&  \
//                        (LABEL(SRC(l_)) || KONST(SRC(l_))) )
{
    int   nivel, em_uso = 0, n = 0, forma = 0;
    TCODE   *lcprox;

    for (; lc && lc->prox; lc = lc->prox)  {

        if (!OP(lc))
           continue;

        lcprox = lc->prox;
/*        if (!lcp->prox)
           break;

        if ( STRCMP(OP(lc), "ld") && STRCMP(DST(lc), reg) &&
             (LABEL(SRC(lc)) || KONST(SRC(lc))) && EPUSH(lcp))
           forma = 8;
        if ( STRCMP(OP(lcp), "ld") && STRCMP(DST(lcp), reg) &&
             (LABEL(SRC(lcp)) || KONST(SRC(lcp))) )
           forma = 16; */

         if (EPUSH(lcprox) && (ELDK16(lc)))  {
           TCODE   *la = lc->prox->prox;
           TCODE   *ldptr = lc;      // Ptr para o LD
           //
           nivel = 1;
           for (; la; la = la->prox)
               if STRCMP(OP(la), "pop")  {
                   nivel--;
                   if (nivel == 0) {
                      // The corresponding pop was found, but can be or not
                      // the same register
                      if STRCMP(DST(la), reg)  {
                         // Remove 'push reg'
                         Del(ldptr->prox);
                         // Remove 'ldptr' from the list
                         ldptr->ante->prox = ldptr->prox;
                         ldptr->prox->ante = ldptr->ante;
                         // Insert ldptr in pop's place
                         la = eins_ele_lst(la, ldptr);
                         Del(la->ante);    // del the POP
                         /*ldp->ante = la->ante;
                         if (la->ante)  la->ante->prox = ldp;
                         ldp->prox = la->prox;
                         if (la->prox)  la->prox->ante = ldp;
                         // Apagar o pop
                         free(la->lbl);  free(la->op);   free(la->src);
                         free(la->dst);  free(la->txt);  free(la);*/
                         n++;
                      }
                      else  {  // Pop is to another reg
                         // Remove 'push reg'
                         Del(ldptr->prox);
                         // Remove ldptr from list
                         ldptr->ante->prox = ldptr->prox;
                         ldptr->prox->ante = ldptr->ante;
                         // Insert ldptr in pop's position, changing the dest
                         // register to the poped reg
                         la = eins_ele_lst(la, ldptr);
                         free(ldptr->dst);
                         ldptr->dst = la->ante->dst;
                         la->ante->dst = NULL;   // Force not-free by DEL...
                         Del(la->ante);   // del POP
                         n++;
                      }
                      break;
                   }
               }
               else if STRCMP(OP(la), "push")
                   nivel++;
               else if (OP(la) == NULL)
                   break;   // the pair must belong to the same C function
        }
    }
    return n;
}



int OG_TrataJP (TCODE *lc)
/* Remove some redundant JP */
{
    TCODE  *au;
    int    n = 0;

    for (; lc && lc->prox; lc = lc->prox)  {

        if (!OP(lc))
           continue;

        if (STRCMP(OP(lc), "jp") || STRCMP(OP(lc), "jr"))
           if ( (lc->prox) && LBL(lc->prox) && STRCMP(SRC(lc), LBL(lc->prox)) )
              {Del(lc);     // Remove the redundant JP (to the next position)
               n++;}
    }
    return n;
}


/** GLOBAL OPTIMIZATION                                                   **/
/***************************************************************************/




/***************************************************************************/
/** LOCAL OPTIMIZATION  (Local optimizations)                             **/

/* Some translations:

   MRef = memory reference
   Igual = equal
   Qualquer = any
   Condicao = condition
   padrao = pattern
   reg8 = 8bit register
   ixMRef = memory reference using Z80 register ix
*/

enum padrao { Label = 1, Konst = 2, MRef = 4, Reg = 8,
              Igual = 16, QualQuer = 32, Reg8 = 64, Reg16 = 128,
              ixMRef = 256, Condicao = 512 };
typedef enum padrao padrao;

typedef struct TPADRAO {
    char    *op;     // Sentinel (NULL signals the end)
    padrao  dst;
    char    *ddst;
    padrao  src;
    char    *dsrc;   // Value, when 'src' demands one
    char    negar;   // 0 or -1, and says if the instr should appear or not
  } TPADRAO;




int avalia_operando (char *op, padrao p, char *subs)
/* Returns 1 iif the operand matches the pattern, else 0 */
{
    switch (p)  {
       case Label :    return LABEL(op);
       case Konst :    return KONST(op);
       case MRef :     return MREF(op);
       case Reg :      return REG(op);
       case Igual :    return STRCMP(subs, op);
       case QualQuer : return 1;
       case Reg8 :     return (strst("abcdehl", op) != NULL) && (strlen(op) == 1);
               // Os espacos entre os regs garantem que regs como fb nao existam
       case Reg16 :    return ((strst("af bc de hl ix iy", op) != NULL)) &&
                              (strlen(op) == 2) && (!strchr(op, ' '));
       case ixMRef :   return (strst(op, "(ix") != NULL);  // Se e' MRef c/ ix
       case Condicao : return (strst("c nc z nz", op) != NULL) && (strlen(op) < 3);
       default :
            ErroI(-61, "Unclassified operand (optimiz:avalia_operando)");
    }
}

cmp_instr_com_padrao (TCODE *i, TPADRAO *ipdr)
/* Returns != 0 iif instr 'i' matches the pattern instr */
{
    // Check if i->op is admissible (if is in ipdr->op)
    if (strst(ipdr->op, i->op) == NULL)
       return 0;
    return avalia_operando(i->src, ipdr->src, ipdr->dsrc) &&
           avalia_operando(i->dst, ipdr->dst, ipdr->ddst);
}


char *get_arg_imco (char *ins, char **arg)
/* Gets the 'n-char' argument of a OMC instr ('s'). Basically returns a
 * string with the chars from '*arg' to the first '.'.
 * Returns a pointer to the next OMC instr, and the string in *arg.
 */
{
    int  i;
    *arg = (char*)meulloc((int)(strchr(ins, '.') - ins) + 2);
    for (i = 0; (ins[i] != '.') && (ins[i]); i++)
        (*arg)[i] = ins[i];
    if (ins[i] != '.')
       ErroI(-54, "String OptimizationMicroInstruction missing '.' terminator");
    (*arg)[i] = '\0';
    if (!i)  *arg = NULL;
    return &ins[i + 1];
}


TCODE *inc_pos (TCODE *c)
{
    // Advance 1 z80 asm instruction, ignoring non asm TCODEs (they
    // ain't counted)
    while (c)  {
       c = c->prox;
       if (OP(c))
          break;    // Para qdo encontrar 1 instr (a proxima instr)
    }
    if (c == NULL)
       ErroI(-51, "OptimizationMicroInstruction with Position out of range");
    return c;
}


TCODE *set_pos (TCODE *c, char pos)
{
    for (pos -= '0'; pos; pos--)
        c = inc_pos(c);
    return c;
}


TCODE *exec_optim_prgm (TCODE *c, char *prgm, int *n_optim)
/* Execute an OMC (Optmization Micro Code) program over TCODE list. 
   Program in prgm. Returns 1 iif the optimization was performed.
*/
/* Executa uma programa numa linguagem especial destinada a modificar
 * uma sequencia de instrucoes apontadas por c. O programa encontra-se
 * na string prgm. Retorna 1 se a optimizacao foi mesmo feita, 0 c.c. */
#define SEL(cc_)  ((cc_ == 'D')? cptr->dst: cptr->src)
#define SELau(cc_)  ((cc_ == 'D')? au->dst: au->src)
#define SELn(n__,cc_)  ((cc_ == 'D')? (n__)->dst: (n__)->src)
{
    TCODE   *cptr = c, *iaux, *au;
    char    *s, *s1, *t;
    int     u;

    (*n_optim)++;
    while (*prgm)
       switch (*prgm)  {
                      // (deletes current instruction)
          case 'A' :  // Apaga a instrucao actual
               au = cptr;  prgm++;
               Del(cptr);
               if (c == au)
                  c = cptr;
               break;
                      // (position the instruction cursor at position x ('0' to '9')
          case 'P' :  // Posicionar o 'cursor de instrucoes' na pos x ('0' a '9')
               cptr = set_pos(c, prgm[1]);
               prgm += 2;
               break;
                      // (inserts an (empty) instruction)
          case 'I' :  // Insere uma nova instrucao, vazia
               iaux = New(TCODE);
               memset(iaux, 0, sizeof(TCODE));
               iaux->op = (char*)1;     // So' para nao ser filtrado por set_pos
               cptr = add_ele_lst(cptr, iaux);
               prgm++;
               break;
                      // (changes opcode of the current instruction (pointed to by the cursor))
          case 'O' :  // Altera o opcode da instr actual (apontada pelo cursor)
               free(cptr->op);
               prgm = get_arg_imco (prgm + 1, &cptr->op);
               break;
                      // (changes src of the current instruction)
          case 'S' :  // Altera o src da instr actual
               free(cptr->src);
               prgm = get_arg_imco (prgm + 1, &cptr->src);
               break;
                      // (changes dst of the current instruction)
          case 'D' :  // Altera o dst da instr actual
               free(cptr->dst);
               prgm = get_arg_imco (prgm + 1, &cptr->dst);
               break;
                      // (makes a copy of one dst/src to another dst/src)
          case 'C' :  // Faz uma copia de um dst/src para outro src/dst
               iaux = cptr;
               cptr = set_pos(c, prgm[2]);     // de
               s = SEL(prgm[1]);
               cptr = set_pos(c, prgm[4]);     // para
               if (prgm[3] == 'D')  { free(cptr->dst); cptr->dst = dupstr(s); }
               else                 { free(cptr->src); cptr->src = dupstr(s); }
               cptr = iaux, prgm += 5;
               break;
                      // (transforms Dst or Src in a memory reference (by envolving it in parentesis))
          case 'M' :  // Transforma Dst ou Src em mem ref (pondo '(' e ')')
               prgm++;
               s = meulloc(strlen(SEL(*prgm)) + 2+1);
               sprintf(s, "(%s)", SEL(*prgm));
               free(SEL(*prgm));
               if (*prgm++ == 'D')  cptr->dst = s;
               else                 cptr->src = s;
               break;
                      // (removes the 1st and last characters from dst/src (removes a memory reference))
          case 'T' :  // Tira o 1§ e ult chars de dst/src (saca mem ref)
               prgm++;
               if (*prgm++ == 'D')  s = cptr->dst;
               else                 s = cptr->src;
               s[strlen(s) - 1] = 0;
               strcpy(s, s+1);
               break;
                      // (appends some text to dst/src)
          case 'J' :  // Junta ao final de dst/src algum texto
               s1 = prgm + 1;
               prgm = get_arg_imco(prgm + 2, &t);
               s = meulloc(strlen(SEL(*s1)) + 2 + strlen(t));
               sprintf(s, "%s%s", SEL(*s1), t);
               free(SEL(*s1));
               if (*s1 == 'D')  cptr->dst = s;
               else             cptr->src = s;
               free(t);
               break;
                      // (concats "ix" to the start of the current D/S)
          case 'X' :  // Concatena "ix" ao inicio do D/S actual
               t = meulloc(2 + strlen(SEL(*++prgm)) + 1);
               sprintf(t, "ix%s", SEL(*prgm));
               free(SEL(*prgm));
               if (*prgm++ == 'D')  cptr->dst = t;
               else                 cptr->src = t;
               break;
                      // (concats an S/D to the end of the current S/D (used for constants)
          case 'E' :  // Concatena um S/D ao fim do S/D actual (usado p/ konsts)
               t = meulloc(64);            // EDD2
               au = set_pos(c, prgm[3]);
               sprintf(t, "%s%s", SEL(prgm[1]), SELau(prgm[2]));
               free(SEL(prgm[1]));
               if (prgm[1] == 'D')  cptr->dst = t;
               else                 cptr->src = t;
               prgm += 4;
               break;
                      // (check if 2 items are equal, if not, no optimizations are performed)
          case 'V' :  // Verifica se 2 items sao iguais. Se nao forem nao se optimiza
               iaux = set_pos(c, prgm[2]);   // a
               au = set_pos(c, prgm[4]);     // b
               if (strcmp(SELn(iaux, prgm[1]), SELn(au, prgm[3])) != 0)
                  return c;
               prgm += 5;
               break;
          case ' ' : prgm++;
                         // (spaces can be user (ignored), for improved readability)
               break;    // Podemos usar ' ' como separador p/ > legibilidade
          default :
               ErroI(-423, "Unknown OptimizationMicroInstrution");
       }
    return c;
}


int Optimiza (TCODE *code, TPADRAO *pdr, char *optim_prgm, int permitir_labels)
/* Search for pattern match and make replacements...
   Returns the number of matches. */
/* Procura sequencias padrao de instrs e faz substituicoes... */
/* Retorna o numero de optimizacoes feitas. */
{
    int   n_optim = 0, ncicl = 0;

    for (; code; code = code->prox, ncicl++)  {
        TPADRAO  *p = pdr;
        TCODE  *a = NULL;  // Inicializa-se p/ o caso de nao se exec o 2do for   (in case the 2nd for loop isn't executed)

        if (ncicl >= 400000)
           ErroI(-43, "deadlock in \"Optimiza()\"");

        for (; code && (!OP(code)); code = code->prox);

        /* Procurar uma sequencia de opcodes q corresponda 'a sequencia padrao */
        /* Search for an opcodes sequence that matches the pattern */
        for (a = code; a && p->op; a = a->prox)  {
            if ((!permitir_labels) && LBL(a))
               break;       // Ignore labels or instrs that are label targets
            if (!OP(a))
               continue;    // Ignore what are not instrs
            if ( (!cmp_instr_com_padrao(a, p)) ^ p->negar )
               break;
            p++;
         }
        if (!a)  break;
        if (p->op)  continue;         // Not found

        // If we got here is because a match was found. Execute the OCM program.
        code = exec_optim_prgm(code, optim_prgm, &n_optim);
    }

    return n_optim;
}


/* Here are described all the optimizations made.
   The first variable is the pattern, the second is the OMC program.
   The comment preceding the variables shows, at the left side, the
   instruction pattern beeing searched (and described by a TPADRAO),
   and at the right side the code after the execution of the OCM code.
 */


/*struct {
    TPADRAO  *pdr;
    char     *pmco;
} optim[] = {   {"push", QualQuer, "", Reg, "", 0},
                 {"pop", Reg, "", QualQuer, "", 0},
                 {NULL,0,NULL,0,NULL,0}              ,
                 "VS0D1 AA"   };  */
// push  r
// pop   r
TPADRAO pdrPUSHPOP[] = { {"push", QualQuer, "", Reg, "", 0},
                         {"pop", Reg, "", QualQuer, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPUSHPOP = "VS0D1 AA";

//0 ld     r, r
TPADRAO pdrLDrr[] = { {"ld", Reg, "", Reg, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDrr = "VS0D0 A";

//0	ld     hl, label
//1	ld     r8, (hl)
//2	inc    hl
//3	ld     h, (hl)
//4	ld     l, r8              ld    hl, (label)
TPADRAO pdrLDhlGLB[] = { {"ld", Igual, "hl", Label, "", 0},
                         {"ld", Reg8, "", Igual, "(hl)", 0},
                         {"inc", Igual, "hl", QualQuer, "", 0},
                         {"ld", Igual, "h", Igual, "(hl)", 0},
                         {"ld", Igual, "l", Reg8, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDhlGLB = "VD1S4  MS P1AAAA";

//0	ld     hl, label
//1	pop    af                 pop   af
//2	ld     (hl), a            ld    (label), a
//3	!inc    hl    (! indica que esta instrucao NAO deve aparecer)   ("!" means this instruction should NOT be there)
TPADRAO pdr8bitSTO[] = { {"ld", Igual, "hl", Label, "", 0},
                         {"pop", Igual, "af", QualQuer, "", 0},
                         {"ld", Igual, "(hl)", Igual, "a", 0},
                         {"inc", Igual, "hl", QualQuer, "", 1},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmco8bitSTO = "CS0D2P2MD P0A";

//0	ld     hl, label
//1	pop    af                 pop  af
//2	ld     (hl), a            ld   (label), a
//3	inc    hl                 xor  a
//4	ld     (hl), 0            ld   (label + 1), a
TPADRAO pdr8bitSTOc[] = { {"ld", Igual, "hl", Label, "", 0},
                          {"pop", Igual, "af", QualQuer, 0, 0},
                          {"ld", Igual, "(hl)", Igual, "a", 0},
                          {"inc", Igual, "hl", QualQuer, "", 0},
                          {"ld", Igual, "(hl)", Konst, "", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmco8bitSTOc = "CS0D2P2MD CS0D4P4JD+1.Sa.MD P3Oxor.Da. P0A";

//0	ld     hl, label
//1	ld     a, (hl)            ld    a, (label)
//2 !inc   hl     (! indica que esta instrucao NAO deve aparecer)   ("!" means this instruction should NOT be there)
TPADRAO pdrLDa[] = { {"ld", Igual, "hl", Label, "", 0},
                     {"ld", Igual, "a", Igual, "(hl)", 0},
                     {"inc", Igual, "hl", QualQuer, 0, 1},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDa = "CS0S1 A MS";

//0	ld     hl, label
//1	pop    bc
//2	ld     (hl), c
//3	inc    hl                 pop   bc
//4	ld     (hl), b            ld    (label), bc
TPADRAO pdrLDbc[] = { {"ld", Igual, "hl", Label, "", 0},
                      {"pop", Igual, "bc", QualQuer, "", 0},
                      {"ld", Igual, "(hl)", Igual, "c", 0},
                      {"inc", Igual, "hl", QualQuer, 0, 0},
                      {"ld", Igual, "(hl)", Igual, "b", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDbc = "CS0D4 P4 Sbc.MD P2AAP0A";

//0	push   hl
//1	ld     hl, ..             ex     de, hl
//2	pop    de                 ld     hl, ..
TPADRAO pdrDeHl[] = { {"push", QualQuer, "", Igual, "hl", 0},
                      {"ld", Igual, "hl", QualQuer, "", 0},
                      {"pop", Igual, "de", QualQuer, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDeHl = "Oex.Dde.Shl. P2A";

//0	push   hl
//1	pop    bc
//2	ld     (..), bc           ld     (..), hl
TPADRAO pdrBcHl[] = { {"push", QualQuer, "", Igual, "hl", 0},
                      {"pop", Igual, "bc", QualQuer, "", 0},
                      {"ld", MRef, "", Igual, "bc", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoBcHl = "AA Shl.";

//0	push   hl
//1	ld     hl, label
//2	pop    bc                 ld    a, l
//3	ld     (hl), c            ld    (label), a
//4 !inc   hl     (! indica que esta instrucao NAO deve aparecer)   ("!" means this instruction should NOT be there)
TPADRAO pdrDhlGLB[] = { {"push", QualQuer, "", Igual, "hl", 0},
                        {"ld", Igual, "hl", Label, "", 0},
                        {"pop", Igual, "bc", QualQuer, "", 0},
                        {"ld", MRef, "", Igual, "c", 0},
                        {"inc", Igual, "hl", QualQuer, "", 1},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDhlGLB = "Old.Da.Sl.  CS1D1 P1MDSa.  P2AA";

//0	ld     hl, label
//1	ld     e, (hl)
//2	inc    hl
//3	ld     d, (hl)            ld    de, (label)
TPADRAO pdrDeGLB[] = { {"ld", Igual, "hl", Label, "", 0},
                       {"ld", Igual, "e", Igual, "(hl)", 0},
                       {"inc", Igual, "hl", QualQuer, "", 0},
                       {"ld", Igual, "d", Igual, "(hl)", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDeGLB = "Dde.MS P1AAA";

//0	ld     hl, ..
//1	ld     a, l               ld     a, ..
//2	ld     (label), a         ld     (label), a
//!	ld     .., h
TPADRAO pdrLDGLBk[] = { {"ld", Igual, "hl", QualQuer, "", 0},
                        {"ld", Igual, "a", Igual, "l", 0},
                        {"ld", MRef, "", Igual, "a", 0},
                        {"ld", QualQuer, "", Igual, "h", 1},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDGLBk = "CS0S1A";

/*************************************************************************/
/*************************************************************************/

//0	push   ix
//1	pop    hl
//2 push   de
//3 ld     de, k
//4	add    hl, de
//5 pop    de
//6	ld     r8, (hl)
//7	inc    hl
//8	ld     h, (hl)            ld    l, (ix + k)
//9	ld     l, r8              ld    h, (ix + k + 1)
TPADRAO pdrLDlptr[] = { {"push", QualQuer, "", Igual, "ix", 0},
                        {"pop", Igual, "hl", QualQuer, "", 0},
                        {"push", QualQuer, "", Igual, "de", 0},
                        {"ld", Igual, "de", Konst, "", 0},
                        {"add", Igual, "hl", Igual, "de", 0},
                        {"pop", Igual, "de", QualQuer, "", 0},
                        {"ld", Reg8, "", Igual, "(hl)", 0},
                        {"inc", Igual, "hl", QualQuer, "", 0},
                        {"ld", Igual, "h", Igual, "(hl)", 0},
                        {"ld", Igual, "l", Reg8, "", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDlptr = "VD6S9  Old.Dl.CS3S0XSMS  P1Old.Dh.CS3S1JS+1.XSMS  P2AA AA AA AA";

//0 push   ix
//1	pop    hl
//2	push   de
//3	ld     de, k
//4	add    hl, de
//5	pop    de
//6 pop    r16                pop    r16
//7	ld     (hl), r8           ld     (ix + k), r8
//8!inc    hl
TPADRAO pdrLV8[] = { {"push", QualQuer, "", Igual, "ix", 0},
                     {"pop", Igual, "hl", QualQuer, "", 0},
                     {"push", QualQuer, "", Igual, "de", 0},
                     {"ld", Igual, "de", Konst, "", 0},
                     {"add", Igual, "hl", Igual, "de", 0},
                     {"pop", Igual, "de", QualQuer, "", 0},
                     {"pop", Reg16, "", QualQuer, "", 0},
                     {"ld", Igual, "(hl)", Reg8, "", 0},
                     {"inc", Igual, "hl", QualQuer, "", 1},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLV8 = "CS3D7 P7XDMD P0AAAAAA";

//0	push   ix
//1	pop    hl       0
//2	push   de       1
//3	ld     de, k    2
//4	add    hl, de   3
//5	pop    de       4
//6	pop    r16      5
//7	ld     (hl), r8 6         pop   hl
//8	inc    hl       7         ld    (ix+k), l
//9	ld     (hl), r8 8         ld    (ix+k+1), h
TPADRAO pdrLV16[] = { {"push", QualQuer, "", Igual, "ix", 0},
                      {"pop", Igual, "hl", QualQuer, "", 0},
                      {"push", QualQuer, "", Igual, "de", 0},
                      {"ld", Igual, "de", Konst, "", 0},
                      {"add", Igual, "hl", Igual, "de", 0},
                      {"pop", Igual, "de", QualQuer, "", 0},
                      {"pop", Reg16, "", QualQuer, "", 0},
                      {"ld", Igual, "(hl)", Reg8, "", 0},
                      {"inc", Igual, "hl", QualQuer, "", 0},
                      {"ld", Igual, "(hl)", Reg8, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLV16 = "A CS2D1 P1Old.Sl.XDMD  CS2D2 P2Old.Sh.XDJD+1.MD  P3AA AA AA";

//0	push   ix
//1	pop    hl
//2	push   de
//3	ld     de, k
//4	add    hl, de
//5	pop    de
//6	pop    af        0
//7	ld     (hl), a   1        pop   af
//8	inc    hl        2        ld    (ix+k), a
//9	ld     (hl), 0   3        ld    (ix+k+1), 0
TPADRAO pdrLV16c[] = { {"push", QualQuer, "", Igual, "ix", 0},
                       {"pop", Igual, "hl", QualQuer, "", 0},
                       {"push", QualQuer, "", Igual, "de", 0},
                       {"ld", Igual, "de", Konst, "", 0},
                       {"add", Igual, "hl", Igual, "de", 0},
                       {"pop", Igual, "de", QualQuer, "", 0},
                       {"pop", Igual, "af", QualQuer, "", 0},
                       {"ld", Igual, "(hl)", Igual, "a", 0},
                       {"inc", Igual, "hl", QualQuer, "", 0},
                       {"ld", Igual, "(hl)", Konst, "", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLV16c = "CS3D7 CS3D9  AAA AAA  P1XDMD  P2A  XDJD+1.MD";

//0 push   ix
//1	pop    hl
//2	push   de
//3	ld     de, k
//4	add    hl, de
//5	pop    de
//6	ld     r8, (hl)           ld     r8, (ix + k)
//7!inc    hl
//8!inc    hl
TPADRAO pdrRV8[] = { {"push", QualQuer, "", Igual, "ix", 0},
                     {"pop", Igual, "hl", QualQuer, "", 0},
                     {"push", QualQuer, "", Igual, "de", 0},
                     {"ld", Igual, "de", Konst, "", 0},
                     {"add", Igual, "hl", Igual, "de", 0},
                     {"pop", Igual, "de", QualQuer, "", 0},
                     {"ld", Reg8, "", Igual, "(hl)", 0},
                     {"inc", Igual, "hl", QualQuer, "", 1},
                     {"inc", Igual, "hl", QualQuer, "", 1},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoRV8 = "CS3S6 P6XSMS  P0AAA AAA";

//0	ld     r8, (ix..)
//1	ld     r81, r8             ld    r81, (ix..)
//2	ld     r82, 0              ld    r82, 0
TPADRAO pdrRV8c[] = { {"ld", Reg8, "", ixMRef, "", 0},
                      {"ld", Reg8, "", Reg8, "", 0},
                      {"ld", Reg8, "", Konst, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoRV8c = "VD0S1 CS0S1 A";

//0	push   ix
//1	pop    hl
//2	push   de
//3	ld     de, k
//4	add    hl, de
//5	pop    de
//6	ld     e, (hl)  0
//7	inc    hl       1         ld    e, (ix+k)
//8	ld     d, (hl)  2         ld    d, (ix+k+1)
TPADRAO pdrRV16[] = { {"push", QualQuer, "", Igual, "ix", 0},
                      {"pop", Igual, "hl", QualQuer, "", 0},
                      {"push", QualQuer, "", Igual, "de", 0},
                      {"ld", Igual, "de", Konst, "", 0},
                      {"add", Igual, "hl", Igual, "de", 0},
                      {"pop", Igual, "de", QualQuer, "", 0},
                      {"ld", Igual, "e", Igual, "(hl)", 0},
                      {"inc", Igual, "hl", QualQuer, "", 0},
                      {"ld", Igual, "d", Igual, "(hl)", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoRV16 = "CS3S6 CS3S8  P0AAA AAA  XSMS  P1A  XSJS+1.MS";

//0 push   hl                  // Aparece qdo "8 = 16"     (comes up on 16bit assignments to 8bits)
//1	pop    bc
//2	ld     (ix..), c          ld     (ix..), l
TPADRAO pdrIXL[] = { {"push", QualQuer, "", Igual, "hl", 0},
                     {"pop", Igual, "bc", QualQuer, "", 0},
                     {"ld", ixMRef, "", Igual, "c", 0},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoIXL = "AA  Sl.";

//0 ld     r8, k
//1	ld     a, ..   )          ld    a, ..
//2	op     r8                 op    k
TPADRAO pdrPoupR8k[] = { {"ld", Reg8, "", Konst, "", 0},
                         {"ld", Igual, "a", QualQuer, "", 0},
                         {"add sub or and xor cp", QualQuer, "", Reg8, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
//0	ld     r8, (ix..)         ld    a, ..
//1	ld     a, ..              op    (ix..)
//2	op     r8
TPADRAO pdrPoupR8ix[] = { {"ld", Reg8, "", ixMRef, "", 0},
                          {"ld", Igual, "a", QualQuer, "", 0},
                          {"add sub or and xor cp", QualQuer, "", Reg8, "", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
//0	ld     r8, (hl)           ld    a, ..
//1	ld     a, ..              op    (hl)
//2	op     r8
TPADRAO pdrPoupR8hl[] = { {"ld", Reg8, "", Igual, "(hl)", 0},
                          {"ld", Igual, "a", QualQuer, "", 0},
                          {"add sub or and xor cp", QualQuer, "", Reg8, "", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPoupR8 = "VD0S2  CS0S2 P0A";

//0	ld     de, k1           // This optim must be applied before transforming
//1	add    hl, de           // "push ix/pop hl/..." in "(ix+..)" because
//2 pop    de               // after is too late
//3	push   de
//4	ld     de, k2             ld    de, k1 + k2
//5	add    hl, de             add   hl, de
TPADRAO pdrPoupLdAdd[] = { {"ld", Igual, "de", Konst, "", 0},
                           {"add", Igual, "hl", Igual, "de", 0},
                           {"pop", Igual, "de", QualQuer, "", 0},
                           {"push", QualQuer, "", Igual, "de", 0},
                           {"ld", Igual, "de", Konst, "", 0},
                           {"add", Igual, "hl", Igual, "de", 0},
                           {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPoupLdAdd = "ESS4  P2AAAA";

//0	sub    +1                 dec   a
TPADRAO pdrSubAdec[] = { {"sub", QualQuer, "", Igual, "+1", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoSubAdec = "Odec. S.Da.";

//0	add    +1                 inc   a
TPADRAO pdrAddAinc[] = { {"add", QualQuer, "", Igual, "+1", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoAddAinc = "Oinc. S.Da.";

//	ld     de, +1
//	ld     hl, ..
//  or     a                  ld   hl, ..
//	sbc    hl, de             dec  hl
TPADRAO pdrSubHLdec[] = { {"ld", Igual, "de", Igual, "+1", 0},
                          {"ld", Igual, "hl", QualQuer, "", 0},
                          {"or", QualQuer, "", Igual, "a", 0},
                          {"sbc", Igual, "hl", Igual, "de", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoSubHLdec = "A P1A Odec. S.";

//	ld     de, +1
//	ld     hl, ..             ld   hl, ..
//	add    hl, de             inc  hl
TPADRAO pdrAddHLinc[] = { {"ld", Igual, "de", Igual, "+1", 0},
                          {"ld", Igual, "hl", QualQuer, "", 0},
                          {"add", Igual, "hl", Igual, "de", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoAddHLinc = "A P1 Oinc. S.";

//	ld     de, +1
//	add    hl, de             inc  hl
TPADRAO pdrIncHL[] = { {"ld", Igual, "de", Igual, "+1", 0},
                       {"add", Igual, "hl", Igual, "de", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoIncHL = "A Oinc. S.";

//0	ld      a, (ix+)
//1	inc/dec a
//2	ld      (ix+), a          inc/dec  (ix+)
TPADRAO pdrIncDec8[] = { {"ld", Igual, "a", ixMRef, "", 0},
                         {"inc dec", Igual, "a", QualQuer, "", 0},
                         {"ld", MRef, "", Igual, "a", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoIncDec8 = "VS0D2 CS0D1  AP1A";

//	ld     de, 1
//0	ld     l, (..)
//1	ld     h, (..)            ld     l, (..)
//2 or     a                  ld     h, (..)
//3	sbc    hl, de             dec    hl
TPADRAO pdrDecLoc16[] = { {"ld", Igual, "de", Konst, "+1", 0},
                          {"ld", Igual, "l", MRef, "", 0},
                          {"ld", Igual, "h", MRef, "", 0},
                          {"or", QualQuer, "", Igual, "a", 0},
                          {"sbc", Igual, "hl", Igual, "de", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDecLoc16 = "AP2A Odec. S.";

//	ld     de, 1
//0	ld     l, (..)            ld     l, (..)
//1	ld     h, (..)            ld     h, (..)
//2	add    hl, de             inc    hl
TPADRAO pdrIncLoc16[] = { {"ld", Igual, "de", Konst, "+1", 0},
                          {"ld", Igual, "l", MRef, "", 0},
                          {"ld", Igual, "h", MRef, "", 0},
                          {"add", Igual, "hl", Igual, "de", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoIncLoc16 = "AP2 Oinc. S.";


//0	ld     a, (_i)
//1	inc/dec   a               ld     hl, _i
//2	ld     (_i), a            inc/dec  (hl)
TPADRAO pdrIncDecGlb8[] = { {"ld", Igual, "a", MRef, "", 0},
                            {"inc dec", Igual, "a", QualQuer, "", 0},
                            {"ld", MRef, "", Igual, "a", 0},
                            {NULL,0,NULL,0,NULL,0}                  };
char *pmcoIncDecGlb8 = "VS0D2 Dhl.TS P2A P1S.D(hl).";

//	ld     l, (ix+)                                   // Appears in the C conditions
//	ld     a, l               ld     a, (ix+)
TPADRAO pdrLDal[] = { {"ld", Igual, "l", ixMRef, "", 0},
                      {"ld", Igual, "a", Igual, "l", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
//	ld     hl, (..)
//	ld     a, l               ld     a, (..)
//  !or    ...
TPADRAO pdrLDal2[] = { {"ld", Igual, "hl", MRef, "", 0},
                       {"ld", Igual, "a", Igual, "l", 0},
                       {"or", QualQuer, "", QualQuer, "", 1},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDal = "CS0S1A";


//0	ld     a, (ix..)
//1	push   af
//2	ld     a, ..              ld    b, (ix..)
//3	pop    bc                 ld    a, ..
TPADRAO pdrOPab[] = { //{"ld", Igual, "a", ixMRef, "", 0},
                      {"push", QualQuer, "", Igual, "af", 0},
                      {"ld", Igual, "a", QualQuer, "", 0},
                      {"pop", Igual, "bc", QualQuer, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
//char *pmcoOPab = "Db.P3AP1A";
char *pmcoOPab = "Old.Db.Sa.P2A";

//	ld     a, +0              xor   a
TPADRAO pdrLDa0[] = { {"ld", Igual, "a", Igual, "+0", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDa0 = "Oxor.S.";

//0	push   af
//1	ld     l, ..              ld     l, ..
//2	ld     h, ..              ld     h, ..
//3	pop    bc                 ld     b, a
TPADRAO pdrLDba[] = { {"push", QualQuer, "", Igual, "af", 0},
                      {"ld", Igual, "l", QualQuer, "", 0},
                      {"ld", Igual, "h", QualQuer, "", 0},
                      {"pop", Igual, "bc", QualQuer, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDba = "AP2 Old.Db.Sa.";

//0	push   af
//1	ld     hl, ..             ld     hl, ..
//2	pop    bc                 ld     b, a
TPADRAO pdrLDbaG[] = { {"push", QualQuer, "", Igual, "af", 0},
                       {"ld", Igual, "hl", QualQuer, "", 0},
                       {"pop", Igual, "bc", QualQuer, "", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDbaG = "AP1 Old.Db.Sa.";

//0	ld     b, a             // Caso tipico que fica apos LDba/LDbaG   (typical case after LDba/LDbaG)
//1	ld     e, b               ld     e, a
//2 ld     d, konst           ld     d, konst  (normalmente e' 0)
TPADRAO pdrDelIND[] = { {"ld", Igual, "b", Igual, "a", 0},
                        {"ld", Igual, "e", Igual, "b", 0},
                        {"ld", Igual, "d", Konst, "", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDelIND = "ASa.";


//0	push   hl
//1	ld     l, ..              ex     de, hl
//2	ld     h, ..              ld     l, ..
//3	pop    de                 ld     h, ..
TPADRAO pdrPuHLPoDE[] = { {"push", QualQuer, "", Igual, "hl", 0},
                          {"ld", Igual, "l", QualQuer, "", 0},
                          {"ld", Igual, "h", QualQuer, "", 0},
                          {"pop", Igual, "de", QualQuer, "", 0},
                          {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPuHLPoDE = "Oex.Dde.Shl. P3A";

//0	ld     a, (ix..)
//1	ld     r8, a              ld     r8, (ix..)
TPADRAO pdrLDc[] = { {"ld", Igual, "a", ixMRef, "", 0},
                     {"ld", Reg8, "", Igual, "a", 0},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDc = "CD1D0 P1A";

//0	ld     b, (ix1..)
//1	ld     a, (ix2..)         ld     a, (ix2..)
//2	cp     b                  cp    (ix1..)
TPADRAO pdrOpCP8[] = { {"ld", Igual, "b", ixMRef, "", 0},
                       {"ld", Igual, "a", ixMRef, "", 0},
                       {"cp", QualQuer, "", Igual, "b", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoOpCP8 = "CS0S2A";

//0	ld     b, (ix..)             // comparacoes  (8 oprel k)     (compares)
//1	ld     a, k               ld     a, k
//2	cp     b                  cp    (ix..)
TPADRAO pdrOpCP82[] = { {"ld", Igual, "b", ixMRef, "", 0},
                        {"ld", Igual, "a", Konst, "", 0},
                        {"cp", QualQuer, "", Igual, "b", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoOpCP82 = "CS0S2A";

//0	ld     b, (ix..)             // exprs com '()'
//1	op     b                  op    (ix..)
TPADRAO pdrOp82b[] = { {"ld", Igual, "b", ixMRef, "", 0},
                       {"add sub xor or and cp", QualQuer, "", Igual, "b", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoOp82b = "CS0S1A";

//0	ld     b, a
//1	ld     a, (ix2..)         // Only (ix..). (label)
//2	add/or/and/xor/cp  b      add/or/and/xor/cp    (ix2..)
TPADRAO pdrOpAB[] = { {"ld", Igual, "b", Igual, "a", 0},
                      {"ld", Igual, "a", ixMRef, "", 0},
                      {"add or xor and cp", QualQuer, "", Igual, "b", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoOpAB = "CS1S2 AA";

//0	ld     de, 1
//1	or     a
//2	sbc    hl, de             dec    hl
TPADRAO pdrDecHL[] = { {"ld", Igual, "de", Igual, "+1", 0},
                       {"or", QualQuer, "", Igual, "a", 0},
                       {"sbc", Igual, "hl", Igual, "de", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoDecHL = "AAOdec.S.";

//	ex     de, hl
//	ld     l, ..              ld     e, ..
//	ld     h, ..              ld     d, ..
//	add    hl, de             add    hl, de
TPADRAO pdrEXout[] = { {"ex", Igual, "de", Igual, "hl", 0},
                       {"ld", Igual, "l", QualQuer, "", 0},
                       {"ld", Igual, "h", QualQuer, "", 0},
                       {"add", Igual, "hl", Igual, "de", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoEXout = "A De.P1Dd.";

//0	ld     l, ..
//1	ld     h, ..              ld     e, ..
//2	ex     de, hl             ld     d, ..
//3	ld     hl, ..             ld     hl, ..
TPADRAO pdrEXout2[] = { {"ld", Igual, "l", QualQuer, "", 0},
                        {"ld", Igual, "h", QualQuer, "", 0},
                        {"ex", Igual, "de", Igual, "hl", 0},
                        {"ld", Igual, "hl", QualQuer, "", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoEXout2 = "De.P1Dd.P2A";

//	ld     hl, label
//	ld     de, k
//	add    hl, de             ld    hl, label + k
TPADRAO pdrLDglbStru[] = { {"ld", Igual, "hl", Label, "", 0},
                           {"ld", Igual, "de", Konst, "", 0},
                           {"add", Igual, "hl", Igual, "de", 0},
                           {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDglbStru = "ESS1 P1AA";

//	ex     de, hl
//	ld     hl, ..             ld     de, ..
//	add    hl, de             add    hl, de
TPADRAO pdrEXout3[] = { {"ex", Igual, "de", Igual, "hl", 0},
                        {"ld", Igual, "hl", QualQuer, "", 0},
                        {"add", Igual, "hl", Igual, "de", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoEXout3 = "A Dde.";

//0	ld     hl, (..)       // Nao ha'perigo de confundir (..) c/ (ix..) pq nao
//1	ex     de, hl             ld    de, (..)          // ha' "ld  hl, (ix..)"
//2	ld     hl, ..             ld    hl, ..
TPADRAO pdrCond16g[] = { {"ld", Igual, "hl", MRef, "", 0},
                         {"ex", Igual, "de", Igual, "hl", 0},
                         {"ld", Igual, "hl", QualQuer, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoCond16g = "Dde.P1A";

//0 ld     l, (ix..)           // Aparece pelo - nas condicoes, 16 == 16
//1	ld     h, (ix..)          ld     e, (ix..)
//2	ex     de, hl             ld     d, (ix..)
//3	ld     l, (ix..)          ld     l, (ix..)
TPADRAO pdrSacaEx[] = { {"ld", Igual, "l", ixMRef, "", 0},
                        {"ld", Igual, "h", ixMRef, "", 0},
                        {"ex", Igual, "de", Igual, "hl", 0},
                        {"ld", Igual, "l", ixMRef, "", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoSacaEx = "De.P1Dd.P2A";

//0	push   af
//1	pop    hl
//2	ld     l, h               ld      l, a
TPADRAO pdrLDindx8[] = { {"push", QualQuer, "", Igual, "af", 0},
                         {"pop", Igual, "hl", QualQuer, "", 0},
                         {"ld", Igual, "l", Igual, "h", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDindx8 = "AASa.";

//0 ld     a, (hl)
//1	inc    hl
//2	ld     h, (hl)
//3	ld     l, a               call   _ldhlind
TPADRAO pdrLdHLind[] = { {"ld", Igual, "a", Igual, "(hl)", 0},
                         {"inc", Igual, "hl", QualQuer, "", 0},
                         {"ld", Igual, "h", Igual, "(hl)", 0},
                         {"ld", Igual, "l", Igual, "a", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLdHLind = "AAA Ocall.D_ldhlind.S.";

//0	push   hl                 ex     de, hl
//1	ld     a, (..)            ld     a, (..)
//2	ld     l, ..              ld     l, ..
//3	ld     h, ..              ld     h, ..
//4	pop    de
TPADRAO pdrPEX[] = { {"push", QualQuer, "", Igual, "hl", 0},
                     {"ld", Igual, "a", MRef, "", 0},
                     {"ld", Igual, "l", QualQuer, "", 0},
                     {"ld", Igual, "h", QualQuer, "", 0},
                     {"pop", Igual, "de", QualQuer, "", 0},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPEX = "Oex.Dde.Shl. P4A";

//0	push   af
//1	pop    bc                 ld     b, a
TPADRAO pdrPop8_2[] = { {"push", QualQuer, "", Igual, "af", 0},
                        {"pop", Igual, "bc", QualQuer, "", 0},
                        {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPop8_2 = "AOld.Db.Sa.";

//	push   hl
//	pop    de                 ex    de, hl
TPADRAO pdrPop16_2[] = { {"push", QualQuer, "", Igual, "hl", 0},
                         {"pop", Igual, "de", QualQuer, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoPop16_2 = "AOex.Shl.";

//0	ld     b, 1           // Este ld b,1 e' redundante
//1	ld     a, ..              ld       a, ..
//2	sla/srl   a               sla/srl  a
TPADRAO pdrTBsh[] = { {"ld", Igual, "b", Igual, "+1", 0},
                      {"ld", Igual, "a", QualQuer, "", 0},
                      {"sla srl", Igual, "a", QualQuer, "", 0},
                      {NULL,0,NULL,0,NULL,0}                  };
char *pmcoTBsh = "A";

//0	ld     bc, k            // provocado pelo caso especial de trocar hl por bc
//1	push   bc               // quando o operando e' uma const de 16 bits
//2	pop    hl               ld   hl, k
TPADRAO pdrKHL[] = { {"ld", Igual, "bc", Konst, "", 0},
                     {"push", QualQuer, "", Igual, "bc", 0},
                     {"pop", Igual, "hl", QualQuer, "", 0},
                     {NULL,0,NULL,0,NULL,0}                  };
char *pmcoKHL = "Dhl.P1AA";

/*** Segunda fase de optimizacoes ***/

//0 ld    r81, r82
//1 ld    r82, r81            ld    r81, r82
TPADRAO pdrLDcir[] = { {"ld", Reg8, "", Reg8, "", 0},
                       {"ld", Reg8, "", Reg8, "", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDcir = "VS0D1 VD0S1 P1A";

//0 ld    hl, k
//1 ld    de, label
//2 add   hl, de              ld    hl, label + k
TPADRAO pdrLDvk8[] = { {"ld", Igual, "hl", Konst, "", 0},
                       {"ld", Igual, "de", Label, "", 0},
                       {"add", Igual, "hl", Igual, "de", 0},
                       {NULL,0,NULL,0,NULL,0}                  };
char *pmcoLDvk8 = "P2A P1 Dhl. ESS0 P0A";




/******** Optimization between consecutive C instructions ********/

/* Eliminate variable reload  */

//0	ld     (ix1..), r81
//1	ld     (ix2..), r82
//2	ld     r81, (ix1..)       ld     (ix1..), r81
//3	ld     r82, (ix2..)       ld     (ix2..), r82
TPADRAO pdrNoReldL16[] =  { {"ld", ixMRef, "", Reg8, "", 0},
                            {"ld", ixMRef, "", Reg8, "", 0},
                            {"ld", Reg8, "", ixMRef, "", 0},
                            {"ld", Reg8, "", ixMRef, "", 0},
                            {NULL,0,NULL,0,NULL,0}                  };
char *pmcoNoReldL16 = "VS0D2 VS1D3  VD0S2 VD1S3  P2AA";

//0	ld     (..), q
//1	ld     q, (..)            ld     (..), q
TPADRAO pdrNoReld[] =  { {"ld", MRef, "", QualQuer, "", 0},
                         {"ld", QualQuer, "", MRef, "", 0},
                         {NULL,0,NULL,0,NULL,0}                  };
char *pmcoNoReld = "VD0S1 VS0D1 P1A";


/** OPTIMIZACAO LOCAL  (Local optimizations)                              **/
/***************************************************************************/

/*
TPADRAO  *pdr[] = {
pdrPUSHPOP,pdrLDrr,pdrLDhlGLB,pdr8bitST,pdr8bitSTOc,pdrLDa,pdrLDbc,pdrDeHl,
pdrBcHl,pdrDhlGLB,pdrDeGLB,pdrLDGLBk,pdrLDlptr,pdrLV8,pdrLV16,pdrLV16c,
pdrRV8,pdrRV8c,pdrRV16,pdrIXL,pdrPoupR8k,pdrPoupR8ix,pdrPoupR8hl,pdrPoupLdAdd,
pdrSubAdec,pdrAddAinc,pdrSubHLdec,pdrAddHLinc,pdrIncHL,pdrIncDec8,pdrDecLoc16,
pdrIncLoc16,pdrIncDecGlb8,pdrLDal,pdrLDal2,pdrOPab,pdrLDa0,pdrLDba,pdrLDbaG,
pdrDelIND,pdrPuHLPoDE,pdrLDc,pdrOpCP8,pdrOpCP82,pdrOp82b,pdrOpAB,pdrDecHL,
pdrEXout,pdrEXout2,pdrLDglbStru,pdrEXout3,pdrCond16g,pdrSacaEx,pdrLDindx8,
pdrLdHLind,pdrPEX,pdrPop8_2,pdrPop16_2,

pdrNoReldL16,pdrNoReld};


char     *pmco[] = {
"VS0D1 AA","VS0D0 A","VD1S4  MS P1AAAA","CS0D2P2MD P0A",
pmco8bitSTOc,pmcoLDa,pmcoLDbc,pmcoDeHl,pmcoBcHl,pmcoDhlGLB,pmcoDeGLB,pmcoLDGLBk,
pmcoLDlptr,pmcoLV8,pmcoLV16,pmcoLV16c,pmcoRV8,pmcoRV8c,pmcoRV16,pmcoIXL,
pmcoPoupR8,pmcoPoupLdAdd,pmcoSubAdec,pmcoAddAinc,pmcoSubHLdec,pmcoAddHLinc,
pmcoIncHL,pmcoIncDec8,pmcoDecLoc16,pmcoIncLoc16,pmcoIncDecGlb8,pmcoLDal,
pmcoOPab,pmcoLDa0,pmcoLDba,pmcoLDbaG,pmcoDelIND,pmcoPuHLPoDE,pmcoLDc,pmcoOpCP8,
pmcoOpCP82,pmcoOp82b,pmcoOpAB,pmcoDecHL,pmcoEXout,pmcoEXout2,pmcoLDglbStru,
pmcoEXout3,pmcoCond16g,pmcoSacaEx,pmcoLDindx8,pmcoLdHLind,pmcoPEX,pmcoPop8_2,
pmcoPop16_2,

pmcoNoReldL16,pmcoNoReld};
*/



void chk (int t, int i, char *s)
{
    if (!t) return;
    if (!i) printf("%8s: 0\t", s);
}/**/


void optimiza_codigo ()
{
    TCODE  *final;
    int    i, t = cmdl.print_optim_stats;

//    print_elista(lst_code, printcode, 1);
    if (cmdl.optimizar == 0)
       return;
    // The optimization's order must be respected
    i = OG_TrataJP(lst_code);                                     chk(t,i,"JP");

    while (i = Optimiza (lst_code, pdrPoupLdAdd, pmcoPoupLdAdd, 1));

    i = Optimiza (lst_code, pdrPUSHPOP, pmcoPUSHPOP, 1);          chk(t,i,"1");
    i = Optimiza (lst_code, pdrLDrr, pmcoLDrr, 1);                chk(t,i,"2");
    i = Optimiza (lst_code, pdr8bitSTO, pmco8bitSTO, 1);          chk(t,i,"4");
    i = Optimiza (lst_code, pdr8bitSTOc, pmco8bitSTOc, 1);        chk(t,i,"5");
    i = Optimiza (lst_code, pdrLDa, pmcoLDa, 1);                  chk(t,i,"6");
    i = Optimiza (lst_code, pdrLDbc, pmcoLDbc, 1);                chk(t,i,"7");

    i = Optimiza (lst_code, pdrLDhlGLB, pmcoLDhlGLB, 1);          chk(t,i,"8");
    i = Optimiza (lst_code, pdrDeHl, pmcoDeHl, 1);                chk(t,i,"9");
    i = Optimiza (lst_code, pdrBcHl, pmcoBcHl, 1);                chk(t,i,"10");
    i = Optimiza (lst_code, pdrDhlGLB, pmcoDhlGLB, 1);            chk(t,i,"11");
    i = Optimiza (lst_code, pdrDeGLB, pmcoDeGLB, 1);              chk(t,i,"12");
    i = Optimiza (lst_code, pdrLDGLBk, pmcoLDGLBk, 1);            chk(t,i,"13");

    i = Optimiza (lst_code, pdrLV8, pmcoLV8, 1);                  chk(t,i,"14");
    i = Optimiza (lst_code, pdrLV16, pmcoLV16, 1);                chk(t,i,"15");
    i = Optimiza (lst_code, pdrLV16c, pmcoLV16c, 1);              chk(t,i,"16");
    i = Optimiza (lst_code, pdrRV8, pmcoRV8, 1);                  chk(t,i,"17");
    i = Optimiza (lst_code, pdrRV8c, pmcoRV8c, 1);                chk(t,i,"18");
    i = Optimiza (lst_code, pdrRV16, pmcoRV16, 1);                chk(t,i,"19");
    i = Optimiza (lst_code, pdrLDlptr, pmcoLDlptr, 1);            chk(t,i,"20");

    i = Optimiza (lst_code, pdrIXL, pmcoIXL, 1);                  chk(t,i,"21");

    i = Optimiza (lst_code, pdrOPab, pmcoOPab, 1);                chk(t,i,"22");
    i = Optimiza (lst_code, pdrPoupR8k, pmcoPoupR8, 1);           chk(t,i,"23");
    i = Optimiza (lst_code, pdrPoupR8hl, pmcoPoupR8, 1);          chk(t,i,"24");
    i = Optimiza (lst_code, pdrPoupR8ix, pmcoPoupR8, 1);          chk(t,i,"25");
    // Now we remove some more push/pop pairs that were made uncovered
    // by the other optimizations. This call shouldn't be removed from
    // the beggining, because it prepares some terrain for other optimizations.
    i = Optimiza (lst_code, pdrPUSHPOP, pmcoPUSHPOP, 1);          chk(t,i,"26");

//    print_elista(lst_code, printcode, 1);
    if (cmdl.optimizar <= 1)
       return;

    i = Optimiza (lst_code, pdrAddHLinc, pmcoAddHLinc, 1);        chk(t,i,"27");
    i = Optimiza (lst_code, pdrSubHLdec, pmcoSubHLdec, 1);        chk(t,i,"28");

    i = OG_PushPopReg(lst_code, "af", 0);                         chk(t,i,"29 OG af");
    i = OG_PushPopReg(lst_code, "bc", 1);                         chk(t,i,"30 OG bc");
    i = OG_PushPopReg(lst_code, "de", 1);                         chk(t,i,"31 OG de");
    i = OG_PushPopReg(lst_code, "hl", 0);                         chk(t,i,"32 OG hl");

    i = Optimiza (lst_code, pdrAddAinc, pmcoAddAinc, 1);          chk(t,i,"33");
    i = Optimiza (lst_code, pdrSubAdec, pmcoSubAdec, 1);          chk(t,i,"34");

    i = OG_PushPopRegComValor(lst_code, "af");                 chk(t,i,"35 OG af");
    i = OG_PushPopRegComValor(lst_code, "bc");                 chk(t,i,"36 OG bc");
    i = OG_PushPopRegComValor(lst_code, "de");                 chk(t,i,"37 OG de");
    i = OG_PushPopRegComValor(lst_code, "hl");                 chk(t,i,"38 OG hl");

    i = Optimiza (lst_code, pdrIncDec8, pmcoIncDec8, 1);          chk(t,i,"39");
    i = Optimiza (lst_code, pdrIncLoc16, pmcoIncLoc16, 1);        chk(t,i,"40");
    i = Optimiza (lst_code, pdrDecLoc16, pmcoDecLoc16, 1);        chk(t,i,"41");
    i = Optimiza (lst_code, pdrIncDecGlb8, pmcoIncDecGlb8, 1);    chk(t,i,"IncDecGlb8");

    i = Optimiza (lst_code, pdrLDal, pmcoLDal, 1);                chk(t,i,"44");
    i = Optimiza (lst_code, pdrLDal2, pmcoLDal, 1);               chk(t,i,"45");

    i = OG_PushPopRegComValor(lst_code, "af");                    chk(t,i,"46 OG af");
    i = OG_PushPopRegComValor(lst_code, "bc");                    chk(t,i,"47 OG bc");
    i = OG_PushPopRegComValor(lst_code, "de");                    chk(t,i,"48 OG de");
    i = OG_PushPopRegComValor(lst_code, "hl");                    chk(t,i,"49 OG hl");

    i = Optimiza (lst_code, pdrPuHLPoDE, pmcoPuHLPoDE, 1);        chk(t,i,"51");
    i = Optimiza (lst_code, pdrLDba, pmcoLDba, 1);                chk(t,i,"52");
    i = Optimiza (lst_code, pdrLDbaG, pmcoLDbaG, 1);              chk(t,i,"LDbaG");
    // DelIND tem que se seguir aos LDba.   (DelIND must be after LDba)
    i = Optimiza (lst_code, pdrDelIND, pmcoDelIND, 1);            chk(t,i,"DelIND");
    i = Optimiza (lst_code, pdrLDc, pmcoLDc, 1);                  chk(t,i,"53");
    i = Optimiza (lst_code, pdrOpCP8, pmcoOpCP8, 1);              chk(t,i,"54");
    i = Optimiza (lst_code, pdrOpCP82, pmcoOpCP82, 1);            chk(t,i,"OpCP82");
    i = Optimiza (lst_code, pdrOp82b, pmcoOp82b, 1);              chk(t,i,"Op82b");
    i = Optimiza (lst_code, pdrOpAB, pmcoOpAB, 1);                chk(t,i,"55");

    i = Optimiza (lst_code, pdrPop8_2, pmcoPop8_2, 1);            chk(t,i,"558");
    i = Optimiza (lst_code, pdrPop16_2, pmcoPop16_2, 1);          chk(t,i,"5516");

    i = Optimiza (lst_code, pdrEXout, pmcoEXout, 1);              chk(t,i,"56");
    i = Optimiza (lst_code, pdrEXout2, pmcoEXout2, 1);            chk(t,i,"57");

    i = Optimiza (lst_code, pdrLDglbStru, pmcoLDglbStru, 1);      chk(t,i,"60");

    i = Optimiza (lst_code, pdrCond16g, pmcoCond16g, 1);          chk(t,i,"61");
    i = Optimiza (lst_code, pdrSacaEx, pmcoSacaEx, 1);            chk(t,i,"62");

    i = Optimiza (lst_code, pdrDecHL, pmcoDecHL, 1);              chk(t,i,"63");
    i = Optimiza (lst_code, pdrLDindx8, pmcoLDindx8, 1);          chk(t,i,"63a");
    // Ultimas optim local interior a fazer
    i = Optimiza (lst_code, pdrKHL, pmcoKHL, 1);                  chk(t,i,"64a");
    i = Optimiza (lst_code, pdrPEX, pmcoPEX, 1);                  chk(t,i,"64");
    i = Optimiza (lst_code, pdrIncHL, pmcoIncHL, 1);              chk(t,i,"64a");

//    i = OG_PushPopRegComValor(lst_code, "bc");                    chk(t,i,"65 OG bc");
//    i = OG_PushPopReg(lst_code, "af", 0);                         chk(t,i,"65aOG af");
    i = OG_MoveLoadRegisto(lst_code, "hl");                       chk(t,i,"65bMLR hl");
    i = Optimiza (lst_code, pdrEXout3, pmcoEXout3, 1);            chk(t,i,"66a'");
    i = Optimiza (lst_code, pdrTBsh, pmcoTBsh, 1);                chk(t,i,"66b'");
    i = Optimiza (lst_code, pdrLDa0, pmcoLDa0, 1);                chk(t,i,"66c");
    // So' sacar "ld  a, (_dd); ld  l, a;  ld  h, 0" depois da seguinte.  (Only remove "ld a, ( ...." after the next one.
    i = Optimiza (lst_code, pdrLDc, pmcoLDc, 1);                  chk(t,i,"66h");

    i = Optimiza (lst_code, pdrLDrr, pmcoLDrr, 1);                chk(t,i,"66x");


    /*** Segunda fase local (2nd phase of local optimizations) ***/
    i = Optimiza (lst_code, pdrLDcir, pmcoLDcir, 1);              chk(t,i,"71");
    i = Optimiza (lst_code, pdrLDvk8, pmcoLDvk8, 1);              chk(t,i,"70");


    /* Optimizaces entre instrucoes C  (optimizations between C statments) */
    // Eliminar reloads seguidos de variaveis  (remove reloads followed by variables)
    i = Optimiza (lst_code, pdrNoReldL16, pmcoNoReldL16, 0);      chk(t,i,"66");
    i = Optimiza (lst_code, pdrNoReld, pmcoNoReld, 0);            chk(t,i,"67");

//    print_elista(lst_code, printcode, 1);
    if (t)  printf("\n");
}



