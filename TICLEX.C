
 /** Gerado por pLexer v1.0 */

 /* This module is the lexical analizer */


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "TICLEX.h"


//#define __TESTING__plexer       // tirar coment para debug

#define Erro(n,msg)     {  if (filep < file_stack)   \
                              printf("%s %i: %s\n", (filep+1)->nome, (filep+1)->nlin, msg); \
                           else                                                     \
                              printf("%s %i: %s\n", filep->nome, filep->nlin, msg); \
                          exit(n); }
#define ErroP(n,msg,p)   { printf("%s %i: ", filep->nome, filep->nlin); \
                           printf(msg,p); exit(n); }




#define MaxNestedFiles   16

static TFINFO file_stack[MaxNestedFiles];  // Pilha de files
TFINFO *filep;   // Esta var e' exportada. Da'info sobre a pos do token actual

char  *inc_dir;    // dir de includes por omissao. se o file dado nao
                   // existir, concatena-se esta string no inicio e
                   // tenta-se de novo




static int isid (int c)
/* Retorna != 0 se c e' valido para um ID */
{
     if (!c)  return 00;
     if (isalpha(c) || isdigit(c) || strchr("_?", c))    return -1;
     else                                                  return 00;
}

static int issimb (int c)
/* Retorna != 0 se c e' 1 simbolo contido nos da tabela de tokens simbolicos */
{
     if (!c)  return 00;
     if (strchr("=!><+-", c))    return -1;
     else                      return 00;
}




/* Tabelas de lexemas */

typedef struct TTABLEX {
    int   tok;
    char  *lex;
  } TTABLEX;


/* Tabela de tokens simbolicos (conj. de simbolos) */
static TTABLEX tsimb[] = {
     { IGUAL, "==" },
     { DIFERENTE, "!=" },
     { MAIOR, ">" },
     { MENOR, "<" },
     { MAIOROUIGUAL, ">=" },
     { MENOROUIGUAL, "<=" },
     { MAISMAIS, "++" },
     { MENOSMENOS, "--" },
     { SETA, "->" },
     { SHL, "<<" },
     { SHR, ">>" },

     { 0, NULL }       /* Sentinela */
  };

/* Tabela de palavras reservadas (comecadas por letras) */
static TTABLEX tpalav[] = {
     { PPINCLUDE, "include" },
     { PPDEFINE, "define" },
     { PPUNDEF, "undef" },
     { PPIFDEF, "ifdef" },
     { PPIFNDEF, "ifndef" },
     { PPENDIF, "endif" },
     { PPERROR, "error" },
     { GETE, "gete" },
     { GETV, "getv" },
     { VOID, "void" },
     { CHAR, "char" },
     { UCHAR, "uchar" },
     { INT, "int" },
     { UINT, "uint" },
     { RETURN, "return" },
     { IF, "if" },
     { IFC, "ifc" },
     { IFNC, "ifnc" },
     { IFZ, "ifz" },
     { IFNZ, "ifnz" },
     { FOR, "for" },
     { ELSE, "else" },
     { WHILE, "while" },
     { TYPEDEF, "typedef" },
     { STRUCT, "struct" },
     { SWITCH, "switch" },
     { BREAK, "break" },
     { CASE, "case" },
     { DEFAULT, "default" },
     { ASM, "asm" },
     { EXTERN, "extern" },
     { FORCE, "force" },
     { EXPORT, "export" },
     { DLL, "dll" },

     { 0, NULL }       /* Sentinela */
  };




int lookstr (char *buf, char *str, int maxchar)
/* Le f para buf ate' ter consumido str (ou EOF) ou ter lido maxchar chars */
{
  int  e = 0;

    *buf = 0;
    for (; ((*buf = fgetc(filep->fin)) != EOF) && (maxchar > 0); buf++, maxchar--) {
       if (*buf == '\n')
          filep->nlin++;

       if (*buf == '\r')  {
          buf--;  continue;
        }

       if (str[e] == *buf)  {
          if (!str[++e])  {
             *++buf = 0;
             return 0;       /* Match! */
           }
        }
       else
          if (e == 1)
             e = (*str == *buf)?  1 :0;
          else
             e = 0;
     }
    return -1;
}




static int getline (char *line, int buf_size)
{
  int     size;

    while (fgets(line, buf_size, filep->fin))  {

     size = strlen(line) - 1;
     line[size--] = '\0';      /* Eliminar o '\n' */
     if (line[size] == '\r')
        line[size] = '\0';     /* Eliminar o '\r' */

     if (!*line)  {               /* Linhas vazias fora, depois de as */
        filep->nlin++;              /* contabilizar */
        continue;
      }

     return 0;
    } /* while */

    if (feof(filep->fin))  return 1;     /* EOF */
    else  return -1;   /* Erro IO */
}



static int lookup (TTABLEX *tab, char *lex, int size)
{
  char  tmp = lex[size];

     lex[size] = 0;
     for (; tab->lex; tab++)
        if (strcmp(tab->lex, lex) == 0)  {
           lex[size] = tmp;
           return tab->tok;
         }

     lex[size] = tmp;
     return 0;
}




void pushf (char *novo)
{
    TFINFO  *au;
    char  nome[1024];

    if (filep == file_stack + MaxNestedFiles - 1)
       ErroP(2, "*** Too many nested includes (max: %i)\n", MaxNestedFiles);

    au = filep + 1;
    au->nlin = 0;

    if (!(au->fin = fopen(novo, "rt")))  {
       sprintf(nome, "%s%s", inc_dir, novo);
       if (!(au->fin = fopen(nome, "rt")))
          ErroP(1, "*** File not found (%s)\n", novo);
       novo = nome;
    }

    strcpy(au->nome, novo);
    filep = au;    // So' agora, para ErroP dar a posicao correcta do include
}


static int popf ()
{
    if (filep < file_stack)
       ErroI(3, "file stack already empty (ticlex.popf())\n\n");
    fclose(filep->fin);
    filep--;
    return (filep < file_stack)? -1: 0;
}



#define MaxUnget    1024
static TTOKEN  pilha[MaxUnget];     // Pilha de tokens.
static int     topo = -1;


void ungettoken (TTOKEN *tok)
{
    topo++;
    assert(topo < MaxUnget);
    pilha[topo] = *tok;
}


char   linha[4096] = {0}, *line = linha, *tok, tmp;



static TTOKEN *_gettoken ()
/* Retorna EOT se EOF */
{
# define Return(r)  { filep->ncol = (line - linha);  return (r); }
  static TTOKEN token;
  static int    aux;

    if (topo >= 0)
       return &pilha[topo--];

    token.token = EOT;
    if (filep == file_stack - 1)     // Se nao ha' toks, return EOT
       return &token;

    for (;;)  {

     /* Obter uma linha, se necessario */
      if (!*line)  {
         while (1)  {
            int  aux = getline(linha, 4096);
            if ((!*linha) || (aux == 1))
               if (popf())
                  return &token;       /* FIM */
               else
                  continue;   // Apos um popf(), ler 1 linha
            break;
         }

         filep->nlin++;
         filep->ncol = 1;
         line = linha;
         token.token = NOVA_LINHA;
         Return(&token);
       }

      for (; isspace(*line); line++);     /* Limpar espacos iniciais */


    /* Comentario ate' ao fim da linha */
      if (!strncmp(line, "\/\/", strlen("\/\/")))  {
         *line = '\0';
         continue;
       }
    /* Comentario de bloco */
      if (!strncmp(line, "/*", strlen("/*")))  {
         if (tok = strstr(line, "*/"))   /* coment acaba nesta linha */
            line = tok + strlen("*/");
         else {                       /* coment nao acaba nesta linha */
           /* Tira todos os chars ate'fim coment */
            if (lookstr(linha, "*/", sizeof(linha)-1))
               Erro(17, "Error reading file");
            *line = '\0';  /* Provoca leitura de nova linha depois de continue */
          }
         if (!*line)  continue;
       }

    /* O codigo daqui p/ baixo assume pelo menos um char em line */
    /* Numeros */
      if (isdigit(*line))  {
         unsigned long ni;
         float  nr;
         char   *i, *f;

         ni = strtoul(line, &i, 0);
       /* So' tenta reais se ha' '.' */
         for (f = line; isdigit(*f); f++);
         if (*f == '.')  nr = (float)strtod(line, &f);
         else  f = line;
       /* O que conseguiu converter mais, ganha. Se empate, e' inteiro. */
         if (i >= f)  {
            token.token = INTEIRO;
            token.u.i = ni;
            line = i;      /* Consumir os chars do numero */
          } else  {
            token.token = NREAL;
            token.u.f = nr;
            line = f;
          }
         Return(&token);
       }
    /* IDs e palavs reserv */
      else if (isalpha(*line) || strchr("_?", *line))  {
         int   size;

         for (tok = line; isid(*line); line++);
         size = (int)(line - tok);
         if (aux = lookup(tpalav, tok, size))
            token.token = aux;
         else  {
            token.token = ID;
            token.u.lexema = (char*)meulloc(size + 1 + 1);
            token.u.lexema[0] = '_';       /* Todos os ids comecam c/ '_' */
            strncpy(&token.u.lexema[1], tok, size);
            token.u.lexema[size + 1] = '\0';
          }
         Return(&token);
       }
    /* String */
      else if (*line == '\"')  {
         int   size;
                              /*** FALTA TRATAR \x ******/
         tok = line + 1;
         do {
           line++;
           if (!(line = strchr(line, '\"')))
              Erro(18, "Unterminated string constant");      /* FALTA o fechar */
         } while (*(line-1) == '\\');
         size = (int)(line - tok);
         token.token = STRING;
         token.u.lexema = (char*)meulloc(size + 1);

         strncpy(token.u.lexema, tok, size);
         token.u.lexema[size] = '\0';
         line++;     /* saltar por cima do delimitador final */
         Return(&token);
       }
    /* Caracter */
      else if (*line == '\'')  {      /*** FALTA TRATAR \x ******/
         if (!strchr(line, '\''))
            Erro(18, "Unterminated character constant");   /* FALTA o fechar */
         token.token = CAR;
         token.u.c = line[1];
         line += 3;
         Return(&token);
       }
    /* Tokens simbolicos */
      else  {
         int   size;

         for (tok = line; issimb(*line); line++);
         size = (int)(line - tok);
         if (aux = lookup(tsimb, tok, size))  {
            token.token = aux;
            Return(&token);
          }
       /* Simbolos isolados */
         else  {
            line = tok;
            token.token = *line++;
            Return(&token);
          }
       }
    }
}


#include "..\tipos\listas.h"

typedef struct TDEF {
    struct TDEF    *ante;
    struct TDEF    *prox;
    char           *name;
    TPLST          corpo;
  } TDEF;

TDEF  *lst_defs = NULL;


int proc_def (char *name, TDEF *def)
{  return (strcmp(name, def->name) == 0);  }

void print_def (TDEF *d)
{  printf("%s", d->name);
   print_lista(d->corpo, printt, 0);
}


int ifcond (TTOKEN *tok, int cond)
// Trata um #ifdef (cond = 0) ou #ifndef (cond != 0)
{
   TDEF   *res;
   int    token = '#';

   tok = _gettoken();    // Obter o nome do define

   if ((tok->token == EOT) || (tok->token != ID))
       Erro(12, "Macro name expected");

   res = proc_ele_elst(lst_defs, tok->u.lexema, proc_def);
   if ( (res && cond) || (!res && !cond) )  {
      // Procurar o #endif. token comeca por ser '#'. Quando for encontrado
      // passa a PPENDIF. Quando for encontrado, encontrou-se o fim.
      for (;;)  {
          tok = _gettoken();
          if (tok->token == EOT)
             Erro(13, (cond)? "Unterminated #ifdef": "Unterminated #ifndef");
          if (tok->token == token)
             if (token == '#')
                token = PPENDIF;
             else  {
                tok = _gettoken();
                break;
             }
          // Libertar o token que e' ignorado
          if (tok->token == ID)
             free(tok->u.lexema);
      }
      return 0;
   }
   else
      return 1;
}



TTOKEN *gettoken ()
/* Aqui chama-se _gettoken() que e' quem faz realmente o trabalho. Esta
 * funcao serve de filtro, para efectuar algum preprocessamento, como
 * #include's.                                                          */
{
#define AGAIN       tok->token = NOVA_LINHA; continue;
    static int  endif_flag = 0;
    TTOKEN  *tok, *token;
    TDEF    *def, *res;

    do {
     tok = _gettoken();
     if (tok->token == EOT)
        if (endif_flag != 0)
           Erro(21, "#endif missing");

     // Se for um ID, ver se e' uma macro
     if (tok->token == ID)  {
        res = proc_ele_elst(lst_defs, tok->u.lexema, proc_def);
        if (res)  {
           TPLST   ite = res->corpo;
           free(tok->u.lexema);
           for (; ite->prox; ite = ite->prox);   // Seek end of list
           for (; ite; ite = ite->ante)          // Go backwards, because is stack
               ungettoken((TTOKEN*)ite->data);
           // Ler ja' o primeiro token
           AGAIN;
        }
     }

     // Verificar directivas do processador
     if (tok->token == '#')  {
        tok = _gettoken();      // Obter o nome do define
        if (tok->token == EOT)
           Erro(19, "Unterminated preprocessor directive");

        switch (tok->token) {
          case PPINCLUDE :
               tok = _gettoken();
               if (tok->token == EOT)
                  Erro(3, "Unexpected end of file...");
               if (tok->token != STRING)
                  Erro(3, "Invalid file name");
               pushf(tok->u.lexema);
               AGAIN;
               break;

          case PPDEFINE : {
               int   ntoks = 0;
               tok = _gettoken();      // nome da macro
               if ((tok->token == EOT) || (tok->token != ID))
                  Erro(3, "Macro name expected");

               def = New(TDEF);
               def->name = tok->u.lexema;

              // Obter o corpo da macro
               def->corpo = NULL;
               for (;;)  {
                  tok = _gettoken();
                  if (!tok || (tok->token == NOVA_LINHA))
                     break;

                  if (tok->token == '\\')  {
                     tok = _gettoken();
                     if (tok->token == NOVA_LINHA)
                        continue;    // Mais uma linha
                  }

                  ntoks++;
                  if (ntoks >= MaxUnget)
                     Erro(7, "Macro body too big");
                  token = New(TTOKEN);
                  *token = *tok;
                  def->corpo = ins_ele_lst(def->corpo, token);
               }

               def->corpo = ini_lst(def->corpo);
               lst_defs = eins_ele_clst(lst_defs, def);
               //print_elista(lst_defs, print_def, 1);
               } break;

          case PPUNDEF :
               tok = _gettoken();
               if (tok->token == EOT)
                  Erro(20, "Macro name expected");
               res = proc_ele_elst(lst_defs, tok->u.lexema, proc_def);
               if (res)  {
                   del_lista(res->corpo);  //****** Nao chega: nao dealoca os lexemas
                   lst_defs = edel_ele_lst(res);
                   lst_defs = ini_lst(lst_defs);
               }
               else
                   printf("WARNING: Macro name not found (#undef)");
               AGAIN;
               break;

          case PPIFDEF :
               endif_flag += ifcond (tok, 0);
               AGAIN;
               break;

          case PPIFNDEF :
               endif_flag += ifcond (tok, 1);
               AGAIN;
               break;

          case PPENDIF :
               if (endif_flag == 0)
                  Erro(14, "#endif without #ifdef or #ifndef")
               else
                  endif_flag--;
               AGAIN;
               break;

          case PPERROR :
               tok = _gettoken();
               if ((tok->token == EOT) || (tok->token != STRING))
                  Erro(20, "STRING expected after macro #error");
               Erro(53, tok->u.lexema);
               break;
        }
     }
    }  while (tok->token == NOVA_LINHA);

    return tok;
}




int lex (char *nome, char *incdir)
{
    inc_dir = incdir;
    filep = file_stack - 1;
    pushf(nome);
    return 0;
}




#ifdef __TESTING__plexer
//#include "TICLEX.d"
main (int argc, char **argv)
{
 TTOKEN  *t;

    argc--, argv++;
    lex(*argv);
    while (1)  {
       t = gettoken();
       if (!t)  {
         printf("\nERRO em gettoken()\n");  return 0;
       }
       else if (t->token == EOT) {
         printf("\nAcabou!\n");  return 0;
       }
       printf("F:%s L:%i C:%i   ", filep->nome, filep->nlin, filep->ncol);
       printt(t);     printf("\n");
    }
}
#endif






