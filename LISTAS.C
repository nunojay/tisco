
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "listas.h"


//#define TESTING_listas       /* Coloca um main() neste file */


#ifdef TESTING_listas
#include <stdio.h>
#endif


#define TL(var)   ((TLST*)(var))      /* Helps casting void pointers */




void *add_ele_lst (void *lista, void *ele)
{
    if (!lista)   return ele;
    if (TL(lista)->ante)  TL(lista)->ante->prox = TL(ele);
    TL(ele)->ante = TL(lista)->ante;
    TL(ele)->prox = TL(lista);
    TL(lista)->ante = TL(ele);
    return ele;
}





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Listas completamente genericas.         *
 * Cada elemento tem um ptr para os dados. */

void *ins_ele_lst (void *lista, void *data)
/* Retorna um ponteiro para o novo elemento. Se lista = NULL */
/* e' porque esta' a ser criada neste momento. Nao aloca mem */
/* para data. Insere o elemento no final da lista.           */
{
  TLST  *lst = (TLST*)malloc(sizeof(void*) * 3);

    if (!lst)   return NULL;        /* no mem */
    lst->ante = lista;
    lst->prox = NULL;
    if (lista)  {
       lst->prox = TL(lista)->prox;
       TL(lista)->prox = lst;
       if (lst->prox)
          lst->prox->ante = lst;
     }
    lst->data = data;
    return lst;
}


void *del_ele_lst (void *elem)
/* Se elem for o 1§ da lista pode-se perder a lista. */
/* Retorna um ponteiro para o prox elemento, ou NULL se lst vazia. */
/* Se o elem era o ultimo, retorna ponteiro para anterior. Nao */
/* liberta a mem do campo data. */
{
  TLST  *tmp;

    if (TL(elem)->ante)  {
       tmp = TL(elem)->ante;
       tmp->prox = TL(elem)->prox;
       free(elem);
       if (tmp->prox)  return tmp->prox;
       else            return tmp;
     }
    if (TL(elem)->prox)  {
      tmp = TL(elem)->prox;
      tmp->ante = NULL;
      free(elem);
      return tmp;
    }
    free(elem);
    return NULL;
}


void *del_lista (TLST *lst)
/* Apaga todos os elementos de uma lista, incluindo o campo data. */
{
    while (lst)  lst = adel_ele_lst(lst);
    return NULL;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Listas genericas, mas que alocam mem automaticamente para os dados. *
 * Cada elemento tem um ptr para os dados.                             */

void *ains_ele_lst (void *lista, void *data, int size)
/* Retorna um ponteiro para o novo elemento. Se lista = NULL  */
/* e' porque esta' a ser criada neste momento. Se data = NULL */
/* o campo data de void e' posto a NULL. Aloca mem para data. */
/* Insere o elemento no final da lista. */
{
  TLST  *lst = (TLST*)malloc(sizeof(void*) * 3);

    if (!lst)   return NULL;        /* no mem */
    lst->ante = lista;
    lst->prox = NULL;
    if (lista)  {
       lst->prox = TL(lista)->prox;
       TL(lista)->prox = lst;
       if (lst->prox)
          lst->prox->ante = lst;
     }

    if (data)  {
       lst->data = (char*)malloc(size);
       if (!lst->data)  {
          free(lst);
          return NULL;        /* no mem */
        }
       memcpy(lst->data, (char*)data, size);
     }
    else
       lst->data = NULL;
    return lst;
}


void *adel_ele_lst (void *elem)
/* Semelhante a del_ele_lst() mas liberta a memoria do campo data */
{
    free(TL(elem)->data);
    return del_ele_lst(elem);
}


void *adel_lista (TLST *lst)
/* Apaga todos os elementos de uma lista. */
{
    while (lst)  lst = adel_ele_lst(lst);
    return NULL;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Listas completamente genericas extendidas.                  *
 * Cada elemento tem: ptr p/ o anterior       OFS 0   (32bits) *
 *                    ptr p/ o prox           OFS 4            *
 *                    campos arbritrarios     OFS 8            *
 * So' interessa q os elementos tenham os 2 1§s ptrs.          */

void *eins_ele_lst (void *lista, void *ele)
/* Retorna um ponteiro para o novo elemento. Assume-se que   */
/* e' usado um tipo na forma struct { *ante, *prox, ... }.   */
/* Isto permite nao ter a indirecao dada por data. Pode-se   */
/* ter quaisquer campos em *ele, desde que os dois primeiros */
/* sejam o ponteiro para o anterior e o para o proximo.      */
/* Os elementos sao inseridos na cauda da lista.             */
/* Nao e' alocada nenhuma memoria.                           */
{
    assert(ele != NULL);
/*    if TL(lista)  {
       TL(ele)->prox = NULL;
       TL(lista)->prox = ele;
       TL(ele)->ante = lista;
     }
    else  {
       TL(ele)->prox = NULL;
       TL(ele)->ante = NULL;
     }*/
    TL(ele)->ante = TL(ele)->prox = NULL;
    if TL(lista)  {
       TL(ele)->prox = TL(lista)->prox;
       TL(ele)->ante = lista;
       if (TL(lista)->prox)
          TL(lista)->prox->ante = ele;
       TL(lista)->prox = ele;
     }
    return ele;
}


void *eins_ele_clst (void *lista, void *ele)
/* Retorna um ponteiro para o novo elemento. Assume-se que   */
/* e' usado um tipo na forma struct { *ante, *prox, ... }.   */
/* Isto permite nao ter a indirecao dada por data. Pode-se   */
/* ter quaisquer campos em *ele, desde que os dois primeiros */
/* sejam o ponteiro para o anterior e o para o proximo.      */
/* Os elementos sao inseridos 'a cabeca da lista.            */
/* Nao e' alocada nenhuma memoria.                           */
{
    assert(ele != NULL);
    if TL(lista)  {
       TL(ele)->prox = lista;
       TL(lista)->ante = ele;
       TL(ele)->ante = NULL;
     }
    else  {
       TL(ele)->prox = NULL;
       TL(ele)->ante = NULL;
     }
    return ele;
}


void *edel_ele_lst (void *elem)
/* Se elem for o 1§ da lista pode-se perder a lista.               */
/* Retorna um ponteiro para o prox elemento, ou NULL se lst vazia. */
/* Se o elem era o ultimo, retorna ponteiro para anterior.         */
/* Liberta a memoria de elem.                                      */
{
  TLST  *p = NULL;

    if (TL(elem)->ante)  {
       p = TL(elem)->ante;
       TL(elem)->ante->prox = TL(elem)->prox;
     }
    if (TL(elem)->prox)  {
       p = TL(elem)->prox;
       TL(elem)->prox->ante = TL(elem)->ante;
     }
    TL(elem)->prox = TL(elem)->ante = NULL;    // Ajuda no debug
    free(elem);
  /* Retorna o prox ou o ante, o que houver, com preferencia para o prox. */
  /* Se nao houver nenhum (elem era elemto unico), retorna NULL.          */
    return p;
}


void *edel_lista (void *lst)
/* Apaga todos os elementos de uma lista. */
{
    while (lst)  TL(lst) = edel_ele_lst(lst);
    return NULL;
}





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void *proc_ele_lst (void *lst, void *data, int (*fp)())
/* Procura na lista um dado elemento que coincida com data. */
/* data pode ser qualquer coisa, pois a interpretacao cabe  */
/* a fp(), uma funcao de procura. E' passado 'a funcao data */
/* e data do elemento actual da lista a ser comparado. */
/* Retorna um ponteiro p/ o elem ou NULL se nenhum */
/* encontrado. fp retorna != 0 se encontrou o elemento. Se */
/* fp = NULL, entao procura-se o elem cujo campo data = data dado. */
{
    if (fp)  {
       for (; lst; TL(lst) = TL(lst)->prox)
          if (fp(data, TL(lst)->data))
             return TL(lst)->data;
     }
    else  {
       for (; lst; TL(lst) = TL(lst)->prox)
          if (data == TL(lst)->data)
             return TL(lst)->data;
     }
    return NULL;
}


void *proc_ele_elst (void *lst, void *data, int (*fp)())
/* Procura na lista um dado elemento que coincida com data. */
/* data pode ser qualquer coisa, pois a interpretacao cabe  */
/* a fp(), uma funcao de procura. E' passado 'a funcao data */
/* e data do elemento actual da lista a ser comparado. */
/* Retorna um ponteiro p/ o elem ou NULL se nenhum */
/* encontrado. fp retorna != 0 se encontrou o elemento. Se */
/* fp = NULL, entao procura-se o elem cujo campo data = data dado. */
{
    if (fp)  {
       for (; lst; TL(lst) = TL(lst)->prox)
          if (fp(data, lst))
             return lst;
     }
    else  {
       for (; lst; TL(lst) = TL(lst)->prox)
          if (data == lst)
             return lst;
     }
    return NULL;
}



void *ini_lst (void *l)
/* Retorna um ponteiro para o primeiro elemento (cabeca) da lista */
{
    if (!l)  return l;
    for (; TL(l)->ante; TL(l) = TL(l)->ante);
    return l;
}


void *gelst (void *elem)      /* get elem lst */
{
    return TL(elem)->data;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* print_lista serve para os 2 primeiros tipos de lista, *
 * print_elista serve para o tipo extendido de lista     */

void print_lista (TPLST lst, void (*print_data)(), int NEWLINE)
/* Aceita uma lista e imprime-a no ecran. Se f = NULL, por cada      */
/* elemento e' impresso o ponteiro data, caso contrario print_data   */
/* e' uma funcao que sera' chamada com o campo data de cada elemento */
/* da lista para o imprimir. Se NEWLINE != 0 imprime um \n no final. */
{
    printf("[");
    for (; lst; lst = lst->prox)  {
        if (print_data)  {
           if (lst->data)    print_data(lst->data);
           else              printf("Null");
         }
        else
           printf("%i", (int)lst->data);
        if (lst->prox)
           printf(",");
     }
    printf("]");
    if (NEWLINE)
       printf("\n");
}


void print_elista (void *lst, void (*print_data)(void*), int NEWLINE)
/* Aceita uma elista e imprime-a no ecran. Se f = NULL, por cada  */
/* elemento e' impresso o seu endereco, caso contrario print_data */
/* e' uma funcao que sera' chamada com o elemento da lista para o */
/* imprimir. Se NEWLINE != 0 imprime um \n no final. */
{
    printf("[");
    for (; lst; TL(lst) = TL(lst)->prox)  {
        if (print_data)
           print_data(lst);
        else
           printf("%i", (int)lst);
        if (TL(lst)->prox)
           printf(",");
     }
    printf("]");
    if (NEWLINE)
       printf("\n");
}









#ifdef TESTING_listas

typedef struct TT {
    struct TT  *ante;
    struct TT  *prox;
    char       *line;
    int        num;
  } TT;

void printTT (TT *t)
{  printf("(%s;%i)", t->line, t->num);  }

void printLINE (char *s)
{  printf("%s", s);  }

main (int argc, char **argv)
{
  TT  *l, *p, *lst;
  TPLST  ls;
  char  *a = "ola", *b = "barnabe'", *c = "caixe", *d = "dado4";

    argc--, argv++;

    printf("<*=------------------------------------------------------=*>\n");

    lst = NULL;
    l = NEW(TT);
    l->line = a;
    l->num = 1;
    lst = eins_ele_lst(lst, l);
    print_elista(lst,printTT,1);

    l = NEW(TT);
    l->line = b;
    l->num = 2;
    lst = eins_ele_lst(lst, l);
    print_elista(lst,printTT,1);

    l = NEW(TT);
    l->line = c;
    l->num = 3;
    lst = eins_ele_lst(lst, l);
    print_elista(lst,printTT,1);

    lst = edel_ele_lst(lst);
    print_elista(lst,printTT,1);       /**/

    ls = NULL;
    ls = ins_ele_lst(ls, a);
    ls = ins_ele_lst(ls, b);
    ls = ins_ele_lst(ls, c);
    ls = ins_ele_lst(ls, d);
    print_lista(ini_lst(ls), printLINE, 1);

    ls = del_ele_lst(ls);
    print_lista(ini_lst(ls), printLINE, 1);
    /**/

    lst = edel_lista(lst);
    print_elista(lst,printTT,1);

    ls = del_lista(lst);
    print_lista(ls,printLINE,1);
    return 0;
}
#endif






