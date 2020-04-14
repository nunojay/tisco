
 // Generic lists
 /** Implementacao para listas */



#ifndef _listas__
#define _listas__


#define NEW(n)      (n*)malloc(sizeof(n))
#define New(n)      (n*)meulloc(sizeof(n))



/* Para quem quiser usar... */
/* Quem nao quiser basta usar um tipo semelhante a este, em que a unica */
/* coisa que muda e' o tipo void de data, e usar esse tipo como o tipo */
/* nativo da lista. */
typedef struct TLST {
    struct TLST  *ante;
    struct TLST  *prox;
    void         *data;
  } TLST;

typedef TLST  *TPLST;



void *add_ele_lst (void *lista, void *ele);


void *ins_ele_lst (void *lista, void *data);
// Append an element to a list, returning a pointer to the new element.
// Does NOT allocate memory for the element.
// If lista == NULL, it's a new list being created.
/* Retorna um ponteiro para o novo elemento. Se lista = NULL */
/* e' porque esta' a ser criada neste momento. Nao aloca mem */
/* para data. Insere o elemento no final da lista.           */

void *del_ele_lst (void *elem);
// If elem is the head, the list can be lost.
// Returns a pointer to the next element, or the previous if no next.
// Doesn't free the ->data field.
/* Se elem for o 1§ da lista pode-se perder a lista. */
/* Retorna um ponteiro para o prox elemento, ou NULL se lst vazia. */
/* Se o elem era o ultimo, retorna ponteiro para anterior. Nao */
/* liberta a mem do campo data. */

void *del_lista (TLST *lst);
// Delete all elements from the list, including the ->data field.
/* Apaga todos os elementos de uma lista, incluindo o campo data. */



void *ains_ele_lst (void *lista, void *data, int size);
/* Retorna um ponteiro para o novo elemento. Se lista = NULL  */
/* e' porque esta' a ser criada neste momento. Se data = NULL */
/* o campo data de void e' posto a NULL. Aloca mem para data. */
/* Insere o elemento no final da lista. */

void *adel_ele_lst (void *elem);
// Similar to del_ele_lst() but frees the ->data field.
/* Semelhante a del_ele_lst() mas liberta a memoria do campo data. */

void *adel_lista (TLST *lst);
/* Apaga todos os elementos de uma lista. */



void *eins_ele_lst (void *lista, void *ele);
// Append an alement to a list. It's assumed the lista type is a struct
// starting with the 2 fields ante (previous) and prox (next).
/* Retorna um ponteiro para o novo elemento. Assume-se que   */
/* e' usado um tipo na forma struct { *ante, *prox, ... }.   */
/* Isto permite nao ter a indirecao dada por data. Pode-se   */
/* ter quaisquer campos em *ele, desde que os dois primeiros */
/* sejam o ponteiro para o anterior e o para o proximo.      */
/* Os elementos sao inseridos na cauda da lista.             */
/* Nao e' alocada nenhuma memoria.                           */

void *eins_ele_clst (void *lista, void *ele);
// Insert an alement at the head of a list. It's assumed the lista type
// is a struct starting with the 2 fields ante (previous) and prox (next).
/* Retorna um ponteiro para o novo elemento. Assume-se que   */
/* e' usado um tipo na forma struct { *ante, *prox, ... }.   */
/* Isto permite nao ter a indirecao dada por data. Pode-se   */
/* ter quaisquer campos em *ele, desde que os dois primeiros */
/* sejam o ponteiro para o anterior e o para o proximo.      */
/* Os elementos sao inseridos 'a cabeca da lista.            */
/* Nao e' alocada nenhuma memoria.                           */

void *edel_ele_lst (void *elem);
/* Se elem for o 1§ da lista pode-se perder a lista.               */
/* Retorna um ponteiro para o prox elemento, ou NULL se lst vazia. */
/* Se o elem era o ultimo, retorna ponteiro para anterior.         */
/* Liberta a memoria de elem.                                      */

void *edel_lista (void *lst);
/* Apaga todos os elementos de uma lista. */



void *proc_ele_lst (void *lst, void *data, int (*fp)());
// Search in list.
/* Procura na lista um dado elemento que coincida com data. */
/* data pode ser qualquer coisa, pois a interpretacao cabe  */
/* a fp(), uma funcao de procura. E' passado 'a funcao data */
/* e data do elemento actual da lista a ser comparado. */
/* Retorna um ponteiro p/ o elem ou NULL se nenhum */
/* encontrado. fp retorna != 0 se encontrou o elemento. Se */
/* fp = NULL, entao procura-se o elem cujo campo data = data dado. */


void *proc_ele_elst (void *lst, void *data, int (*fp)());
/* Procura na lista um dado elemento que coincida com data. */
/* data pode ser qualquer coisa, pois a interpretacao cabe  */
/* a fp(), uma funcao de procura. E' passado 'a funcao data */
/* e o elemento actual da lista a ser comparado.            */
/* Retorna um ponteiro p/ o elem ou NULL se nenhum          */
/* encontrado. fp retorna != 0 se encontrou o elemento. Se  */
/* fp = NULL, entao procura-se o elem igual a data dado.    */


void *ini_lst (void *l);
// Initializes a list and return a pointer to the head element.
/* Retorna um ponteiro para o primeiro elemento (cabeca) da lista */

void *gelst (void *elem);
// Returns the ->data field.
/* get elem lst. Retorna o campo data. */


/* print_lista serve para os 2 primeiros tipos de lista, *
 * print_elista serve para o tipo extendido de lista     */

void print_lista (TPLST lst, void (*print_data)(), int NEWLINE);
/* Aceita uma lista e imprime-a no ecran. Se f = NULL, por cada      */
/* elemento e' impresso o ponteiro data, caso contrario print_data   */
/* e' uma funcao que sera' chamada com o campo data de cada elemento */
/* da lista para o imprimir. Se NEWLINE != 0 imprime um \n no final. */
/* Ajuda no DEBUG. */

void print_elista (void *lst, void (*print_data)(void*), int NEWLINE);
/* Aceita uma elista e imprime-a no ecran. Se f = NULL, por cada  */
/* elemento e' impresso o seu endereco, caso contrario print_data */
/* e' uma funcao que sera' chamada com o elemento da lista para o */
/* imprimir. Se NEWLINE != 0 imprime um \n no final. */
/* Ajuda no DEBUG. */


#endif

