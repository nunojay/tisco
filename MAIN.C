
 /* Main module */


#include <time.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dir.h>
#include "ticlex.h"
#include "listas.h"
#include "tipos.h"



/* After the parsing, these lists keep a tree representation of the
  parsed program */
TLST  *lst_tipos = NULL;     // Type's list
TLST  *lst_gvars = NULL;     // Static (global) variable's list
TLST  *lst_funcs = NULL;     // Func's list
TLST  *lst_strings = NULL;   // String's list
TDLL  *lst_dlls = NULL;      // Used DLLs name's list


TOPT    cmdl;     // Cmd line options
TSTATS  stats;    // Statistics





void ErroF (int cod, char *msg)
{
   printf("ERROR: %s\n", msg);
   exit(cod);
}


void ErroI (int cod, char *msg)
{
   printf("INTERNAL ERROR: %s\n", msg);
   exit(cod);
}



void *meulloc (int size)
{
    void *p = malloc(size);
    if (!p)  ErroF(-1, "Out of memory");
    stats.mem_total_alocada += size;
    return p;
}


char *dupstr (char *str)
{
    return strcpy(meulloc(strlen(str)+1), str);
}

int cmpstr (char *a, char *b)
{
    if (!(a || b))
       return 0;
    if (a == NULL)
       return -1;
    if (b == NULL)
       return 1;
    return strcmp(a, b);
}



void usage_and_exit ()
{
    printf("Usage: tisco <C_file_name> [-o[0,1,2]] [-s] [-d] [-p] [-i<dir>]"
           "[-ja,-jr,-jo] [-h]\n");
    exit(1);
}


void read_cmdl (int argc, char **argv, TOPT *cml)
{
    int   c;
    char getopt(int, char**, char*), *au;
    extern char *optarg;
    extern int  optind, opterr;
    extern char optopt;

    argc--, argv++;

    if (argc == 0)
       usage_and_exit();

    if (**argv != '-')
       strcpy(cml->src_file, argv[0]);
    else
       usage_and_exit();

    opterr = 0;
    while ((c = getopt(argc, argv, "o:j:i:shprdlza")) != -1)
      switch (c)   {
        case 'o': if (!strcmp(optarg, "0")) cml->optimizar = 0;  // Nao optimizar
                  else if (!strcmp(optarg, "1")) cml->optimizar = 1; // nivel 1
                  else if (!strcmp(optarg, "2")) cml->optimizar = 2; // nivel 2
                  break;
        case 'j': if (!strcmp(optarg, "r"))
                     strcpy(cml->tjmp, "jr");
                  else if (!strcmp(optarg, "a"))
                     strcpy(cml->tjmp, "jp");
                  else if (!strcmp(optarg, "o"))
                     strcpy(cmdl.retjmp, "jp");
                  break;
        case 'i': strcpy(cml->include_dir, optarg);
                  au = strchr(cml->include_dir, 0) - 1;
                  if ((*au != '\\') && (*au != '/'))
                     {  *++au = '\\'; *++au = '\0';  }
                  break;
        case 's': cml->print_stats = TRUE;
                  break;
        case 'l': cml->tipo_prog = tpOVERLAY;
                  cml->gen_args_code = TRUE;
                  break;
        case 'd': cml->tipo_prog = tpDLL;
                  strcpy(cml->tjmp, "jr");
                  break;
        case 'r': cml->reduce_init = TRUE;
                  break;
        case 'a': cml->gen_args_code = TRUE;
                  break;
        case 'p': cml->simple_prgm = TRUE;    // small
                  break;
        case 'z': cml->print_optim_stats = TRUE;
                  break;
        case '?':
             printf("** Unknown option '%c'\n", optopt);
        case 'h' :
             usage_and_exit();
      }

    // Verificar a consistencia das opcoes
//    if ( (cml->tipo_prog == tpDLL) && (strcmp(cml->tjmp, "jp") == 0) )
//          ErroF(-73, "Absolute jumps can't be used with DLLs");
}



main (int argc, char **argv)
{
  TROTNUM  rotinas_numericas;
  float    parsing_end_time, geracao_end_time,
           limpeza_end_time, optimizacao_end_time, end_time;
  int      len;
  char     *s;
  char     d[MAXDRIVE], p[MAXDIR], f[MAXFILE], e[MAXEXT];


/*    char **v = argv;printf("**Tisco00\n");
    for (; *v; v++)
        printf("%s\n", *v); /**/

    printf("\nTISCO - TI Simple C Compiler\nCopyright (c) NSJ aka Viriato  1998\n");

    memset(&stats, 0, sizeof(TSTATS));
    // command line por omissao
    strcpy(cmdl.src_file, "");
    fnsplit(argv[0], d, p, f, e);
    *strrchr(p, '/') = 0;       // Eliminar o \ final
    if (strrchr(p, '\\'))
       *strrchr(p, '\\') = 0;      // Eliminar o \ antes do BIN final
    fnmerge(cmdl.include_dir, d, p, NULL, NULL);
    strcat(cmdl.include_dir, "include\\");
    cmdl.optimizar = 2;
    cmdl.tipo_prog = tpAPP;
    cmdl.print_stats = FALSE;
    cmdl.print_optim_stats = FALSE;
    cmdl.reduce_init = FALSE;
    cmdl.simple_prgm = FALSE;
    cmdl.estimar_tjmp = FALSE;
    strcpy(cmdl.tjmp, "jr");
    strcpy(cmdl.retjmp, "jr");
    cmdl.gen_args_code = FALSE;

    read_cmdl(argc, argv, &cmdl);
//      printf("final: %s\n", cmdl.include_dir);

    clock();
    lex(cmdl.src_file, cmdl.include_dir);

    parse_tic(cmdl.src_file, EOT);
    parsing_end_time = clock();

//    if ((lst_gvars) && (cmdl.tipo_prog == tpDLL))
//       ErroF(-53, "Sorry, no global variables in DLLs...");
    if ((lst_dlls) && (cmdl.tipo_prog == tpDLL))
       ErroF(-57, "Sorry, no DLLs in DLLs...");

    limpa_rotinas_nao_usadas(cmdl.tipo_prog);
    limpeza_end_time = clock();

    // Remove the C filename's extension
    len = strlen(cmdl.src_file);
    s = strrchr(cmdl.src_file, '.');
    if (len - (int)(s - cmdl.src_file) < 3)
       *s = '\0';

    gera_codigo_Z80(&rotinas_numericas);
    geracao_end_time = clock();

    optimiza_codigo();
    optimizacao_end_time = clock();

    wrt_code(&rotinas_numericas);
    end_time = clock();

    printf("File: %s  (%i lines)\n", filep[1].nome, filep[1].nlin);

    if (cmdl.print_stats)  {
       printf(" Total 'mallocated' memory: %5.1fKb\n",
                 (float)stats.mem_total_alocada / 1024);
       printf("        Parsing Time: %3.2f s\n", parsing_end_time / CLOCKS_PER_SEC);
       printf("       Cleaning Time: %3.2f s\n",
                  (limpeza_end_time - parsing_end_time) / CLOCKS_PER_SEC);
       printf("     Generation Time: %3.2f s\n",
                  (geracao_end_time - limpeza_end_time) / CLOCKS_PER_SEC);
       printf("   Optimization Time: %3.2f s\n",
                  (optimizacao_end_time - geracao_end_time) / CLOCKS_PER_SEC);
       printf("     Asm Output Time: %3.2f s\n",
                  (end_time - optimizacao_end_time) / CLOCKS_PER_SEC);
       printf(" Total compile time: %3.2f s\n", end_time / CLOCKS_PER_SEC);
    }

    printf("\n");
    return 0;
}

