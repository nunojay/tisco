
  /*> wrt_code.c <*/

  /*  Picks the TCODE list and writes the asm file, for TASM assembling.
      Exports void wrt_code (TROTNUM *rn)
   */


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "tipos.h"
#include "listas.h"


extern TOPT   cmdl;           // Opcoes  (options)

extern TCODE  *lst_code;      // Lista com o codigo gerado    (generated code list)
extern TLST   *lst_gvars;     // Lista de variaveis globais   (global variables list)
extern TLST   *lst_strings;   // Lista de strings             (strings list)
extern TLST   *lst_funcs;     // Lista de funcoes             (functions list)
extern TDLL   *lst_dlls;      // Lista de DLLs usadas         (used DLL list)

static FILE  *fout;           // Ficheiro de saida (texto asm)    (output file (ASM))



static void wrt (char *fmt, ...)
{
    va_list  ap;
    char   *p, *sval;
    int    ival;

    va_start(ap, fmt);
    for (p = fmt; *p; p++)  {
        if (*p != '%')  {
           fputc(*p, fout);
           continue;
        }
        switch (*++p)  {
           case 'i' : fprintf(fout, "%i", va_arg(ap, int));      break;
           case 'u' : fprintf(fout, "%u", va_arg(ap, uint));     break;
           case 's' : fprintf(fout, va_arg(ap, char*));          break;
           case 'o' :       // 'o' de 'opcode'    ('o' means 'opcode')
                fputc('\t', fout);
                for (ival = 0, sval = va_arg(ap, char*); *sval; sval++, ival++)
                    fputc(*sval, fout);
                for (; ival < 7; ival++)
                    fputc(' ', fout);
                break;
           case 'v' :       // 'v' de 'variavel'   ('v' means "variable")
                fprintf(fout, "%10s", va_arg(ap, char*));
                break;
           default : fputc(*p, fout);
        }
    }
    va_end(ap);
}


static void put_db (int n)
{
    if (!n) return;
    wrt(".db  ");
    for (; n > 1 ; n--)
        wrt("0, ");
    wrt("0\n");
}


static write_gvars ()
// Generates the necessary .db lines for the global variables.
/* Gera os db's necessarios para as variaveis globais */
{
    TPLST   l = lst_gvars;
    int     i, n;
    char    buf[128];

    wrt("\n;; Global variables ;;\n\n");
    for (; l; l = l->prox)  {
        TITEM   *v = (TITEM*)l->data;
        if (v->classe == ciGLBEXTRN)
           continue;        // Ignoram-se declaracoes externas    (ignore external declarations)
        sprintf(buf, "%s:", v->nome);
        fprintf(fout, "%-18s", buf);
        i = v->tipo->size;
        for (;i > 0; i -= 16)  {
            if (i != v->tipo->size)  fprintf(fout, "%18s", "");
            if (i >= 16) put_db(16);
            else         put_db(i);
        }
    }
}


static write_strings ()
// Generate .db for the strings.
/* Gera os db's necessarios para as strings */
{
    TPLST   l = lst_strings;

    wrt("\n;; Strings ;;\n\n");
    for (; l; l = l->prox)  {
        char   *v = (char*)l->data;
        wrt("ST%u: .text \"%s\"\n", (uint)v, v);
        wrt("\t.db 0\n");
    }
}


static write_funcoes ()
/* Escreve as instrucoes das funcoes no ficheiro.
 * O TASM nao gosta do sinal '+' antes de um numero, por isso   (TASM doesn't like '+' on constants, we remove them here)
 * temos que os retirar aqui.       */
#define SEMMAIS(s_)    ((*s_ == '+')? ((s_)+1): s_)
{
    TCODE  *ls;
    char   *s, *d, *a;

    for (ls = lst_code; ls; ls = ls->prox)  {
        if (ls->lbl)
           wrt("\n%s:", ls->lbl);

        s = ls->src;
        d = ls->dst;

        if (ls->op)  {
           wrt("%o", ls->op);
           if (strstr("calls call jp jr", ls->op))  {
              a = s; s = d; d = a;
           }
        }

        if (d)    wrt("%s", d);
        if (s)
           if (d)   wrt(", %s", SEMMAIS(s));
           else     wrt("%s", SEMMAIS(s));

        if (ls->txt)
           wrt("%s", ls->txt);
        else
           wrt("\n");
    }
}


static write_funcoes_lib (TROTNUM *rn)
// Aritmetic assembly routines.
// As rotinas aritmeticas necessarias sao escritas no ficheiro.
{
    wrt("\n;; Basic numerical routines ;;\n\n");

    if (rn->mul16 || rn->mul8)
       fprintf(fout, "%s",
         "_HLumulDE:\n"
         "    push    bc\n"
         "    push    af\n"
         "\n"
         "    ld      b, 16\n"
         "    ld      a, h\n"
         "    ld      c, l\n"
         "    ld      hl, 0\n"
         "HLumulDE_loop:\n"
         "    add     hl, hl\n"
         "    rl      c\n"
         "    rla\n"
         "    jr      nc, HLumulDE_again\n"
         "    add     hl, de\n"
         "HLumulDE_again:\n"
         "    djnz    HLumulDE_loop\n"
         "\n"
         "    pop     af\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");
    if (rn->mul8)
       fprintf(fout, "%s",
         "_AumulB:\n"
         "    push    hl\n"
         "    push    de\n"
         "    ld      h, 0\n"
         "    ld      l, a\n"
         "    ld      d, h\n"
         "    ld      e, b\n"
         "    call    _HLumulDE\n"
         "    ld      a, l\n"
         "    pop     de\n"
         "    pop     hl\n"
         "    ret\n"
         "\n\n");

    if (rn->div16 || rn->div8 || rn->mod8 || rn->mod16)
       fprintf(fout, "%s",
         "_HLudivDE:\n"        // resto sai em de
         "    push    bc\n"
         "    push    af\n"
         "\n"
         "    ld      c, l\n"
         "    ld      b, h\n"
         "    ld      hl, 0\n"
         "    ld      a, 16\n"
         "\nHLudivDE_loop:\n"
         "    sla     c\n"
         "    rl      b\n"
         "    adc     hl, hl\n"
         "    or      a\n"
         "    sbc     hl, de\n"
         "    jr      c, _HLudivDEl2\n"
         "    set     0, c\n"
         "    jr      HLudivDE_segue\n"
         "_HLudivDEl2:\n"
         "    add     hl, de\n"
         "\nHLudivDE_segue:\n"
         "    sub     1\n"
         "    jr      nz, HLudivDE_loop\n\n"
         "    ex      de, hl\n"
         "    ld      l, c\n"
         "    ld      h, b\n"
         "\n"
         "    pop     af\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");
    if (rn->div8)
       fprintf(fout, "%s",
         "_AudivB:\n"
         "    push    hl\n"
         "    push    de\n"
         "    ld      h, 0\n"
         "    ld      l, a\n"
         "    ld      d, h\n"
         "    ld      e, b\n"
         "    call    _HLudivDE\n"
         "    ld      a, l\n"
         "    pop     de\n"
         "    pop     hl\n"
         "    ret\n"
         "\n\n");

    if (rn->mod8)
       fprintf(fout, "%s",
         "_AumodB:\n"
         "    push    hl\n"
         "    push    de\n"
         "    ld      h, 0\n"
         "    ld      l, a\n"
         "    ld      d, h\n"
         "    ld      e, b\n"
         "    call    _HLudivDE\n"
         "    ld      a, e\n"
         "    pop     de\n"
         "    pop     hl\n"
         "    ret\n"
         "\n\n");
    if (rn->mod16)
       fprintf(fout, "%s",
         "_HLumodDE:\n"
         "    call    _HLudivDE\n"
         "    ex      de, hl\n"
         "    ret\n"
         "\n\n");

    if (rn->shr8)
       fprintf(fout, "%s",
         "_AshrB:\n"
         "    push    bc\n"
         "    srl     a\n"
         "    djnz    $-2\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");
    if (rn->shl8)
       fprintf(fout, "%s",
         "_AshlB:\n"
         "    push    bc\n"
         "    sla     a\n"
         "    djnz    $-2\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");

    if (rn->shr16)
       fprintf(fout, "%s",
         "_HLshrDE:\n"
         "    push    bc\n"
         "    ld      b, e\n"
         "    srl     l\n"
         "    rl      h\n"
         "    djnz    $-4\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");
    if (rn->shl16)
       fprintf(fout, "%s",
         "_HLshlDE:\n"
         "    push    bc\n"
         "    ld      b, e\n"
         "    add     hl, hl\n"
         "    djnz    $-1\n"
         "    pop     bc\n"
         "    ret\n"
         "\n\n");

    return 0;
}


static void write_dllusing_code ()
{
    TDLL   *dl;
    int    num_dll = 0;

    if (!lst_dlls)
       return;

    wrt("\n\n;;*;*;*; DLL STUBS ;*;*;*;;\n");
    // Escrever os stubs
    for (dl = lst_dlls; dl; dl = dl->prox, num_dll++)  {
        TPLST  fns = dl->protos;

        for (; fns; fns = fns->prox)  {
            TFUNCAO  *fn = (TFUNCAO*)fns->data;
            if (!fnOUTPUT(*fn))
               continue;       // Funcao da DLL nao foi usada

            wrt("\n%s:\n", fn->nome);
            wrt("   call   Back_Patch\n");
            // *** ATE' 128 DLLs e FUNCOES/DLL ***    (**** up to 128 DLLs and functions per DLL ****
             // *2 pq indexam tabelas de words. Multiplicando ja'
             // poupam-se instrucoes no code final (Back_Patch)
            wrt("   .db   %i, %i\n", num_dll * 2, fn->index * 2);
        }
    }

    wrt("\n");
    {  // Ler o codigo para fazer back-patch e carregar uma DLL   (load the Back_Patch code and DLL load code)
       FILE   *fi;
       char   line[2048];

       if (!(fi = fopen ("dll_code.asm", "rt")))
          ErroF(-133, "dll_code.asm is missing; it's needed when using DLLs");

       while (fgets(line, 2048, fi))
           wrt(line);
       fclose(fi);
       wrt("\n");
    }
}


static void build_routine_table ()
{
    TCODE  *ls;
    TLST   *l;
    int    index = 0, done;

    wrt("\n ;; Rotine Address Table (RAT) ;;\n\n");
    wrt("_routine_address_table:\n");
    index = 0;
    do {
        done = 0;
        for (l = lst_funcs; l; l = l->prox)  {
            TFUNCAO  *fn = (TFUNCAO*)l->data;
            if (!fnEXPORT(*fn))
               continue;
            if (fn->index == index)  {
               fprintf(fout, "   .dw   %-22s   ; %3i\n", fn->nome, index++);
               done = 1;
               break;
            }
        }
    } while (done);

    wrt("\n\n");
}


static void gen_data_relocation_labels ()
// Generate labels for the isntructions that need relocation.
// Gerar labels de recolocacao em instrucoes que precisem deles.
{
#   define S(_c)  ((_c).src)
#   define D(_c)  ((_c).dst)
#   define TEM_SRCGVAR(c)  ((S(c)[0] == '_') || (S(c)[0] == 'S') || (S(c)[1] == '_'))
#   define TEM_DSTGVAR(c)  ((D(c)[0] == '_') || (D(c)[0] == 'S') || (D(c)[1] == '_'))
    int proc_item_fn (TITEM *a, TITEM *b);   // esta' em rotaux.c
    char *getrlbl(uint i);     // esta' em geraz80.c
    TCODE  *ls;

    // Recolocar gvars e jp's, que consiste em gerar labels em     (global vars and jp)
    // instrucoes que usam enderecos absolutos.
    for (ls = lst_code; ls; ls = ls->prox)  {
        TITEM  pr, *it;

        if (ls->op && (strcmp(ls->op, "jp") == 0))  {
           ls->lbl = dupstr(getrlbl((uint)ls));   // Criar o label para a relocacao
           continue;
        }

       // So' aceitamos instr com src e dst. Assim evitamos olhar
       // para "call _..." e simplificamos as macros que ja' nao teem
       // que testar src e dst para ver se existem
        if (!(ls->src && ls->dst))
           continue;
        if (!(TEM_SRCGVAR(*ls) || TEM_DSTGVAR(*ls)))
           continue;

        if TEM_SRCGVAR(*ls)
           pr.nome = dupstr(ls->src);
        else
           pr.nome = dupstr(ls->dst);

        if (*pr.nome == '(')    // Extrair o nome da var
           ++pr.nome;
        // Se for uma string e' sempre recolocada.    (strings are always relocated)
        if (*pr.nome == 'S')  {
           free(pr.nome);
           ls->lbl = dupstr(getrlbl((uint)ls));   // Criar o label para a relocacao
        }
        // Se for uma ref 'a mem ou tiverem sido adicionadas konsts,
        // isto isola o nome da var.
        pr.nome[strcspn(pr.nome, "+-)")] = '\0';

        // Se o label for uma var externa (ou desconhecida!...) nao e' recolocada.   (external or unknown variables are never relocated)
        it = proc_ele_lst(lst_gvars, &pr, proc_item_fn);
        if ( (it->classe == ciGLBEXTRN) || (!it) )
           continue;
        free(pr.nome);

        ls->lbl = dupstr(getrlbl((uint)ls));   // Criar o label para a relocacao
    }
#   undef D           // so' para jogar pelo seguro...    (playing safe...)
#   undef S
}


static void build_relocation_table ()
// Build the RT from the relocation labels.
// Construir a RT a partir dos labels de recolocacao
{
#   define S(_c)  ((_c).src)
#   define D(_c)  ((_c).dst)
#   define MRef(_s)   ((_s) && (*_s == '('))
#   define HLouA(_s)  ((strcmp(_s, "hl") == 0) || (strcmp(_s, "a") == 0))
    char *getrlbl(uint i);     // esta' em geraz80.c    (from geraz80.c)
    TCODE  *ls;
    int    index = 0;

    wrt("\n ;; Relocation Table (RT) ;;\n\n");
    wrt("_relocation_table:\n");
    // Recolocar enderecos absolutos.     (relocated absolute addresses)
    // The entire code (except user asm) is searched for labels begining by S or R,
    // which are strings and relocation labels, and adds them to the relocation table (RT).
    // Todo o codigo (nao o user asm) e' pesquisado em busca de labels
    // comecados por 'R' ou S, que indicam labels de recolocacao e de
    // strings, e coloca-os na RT.
    for (ls = lst_code; ls; ls = ls->prox)

        if ( (ls->lbl) && (*ls->lbl == 'R') )
           if (strstr("callsjp", ls->op))  // Aceita call, calls e jp
              fprintf(fout, "   .dw   %-22s+1\n", ls->lbl);  // call
           else
            // Os +1 ou +2 que aparecem e' devido a algumas instrs de load
            // de um reg de 16 bits da mem teem 1 ou 2 bytes de opcode.
             if (TEM_SRCGVAR(*ls))
                fprintf(fout, "   .dw   %-22s%s\n", ls->lbl,
                              (HLouA(ls->dst) || !MRef(ls->src))? "+1": "+2");
             else if (TEM_DSTGVAR(*ls))
                fprintf(fout, "   .dw   %-22s%s\n", ls->lbl,
                              (HLouA(ls->src) || !MRef(ls->dst))? "+1": "+2");

    fprintf(fout, "   .dw   %-22s   ; Table end\n", "0");
    wrt("_reloc_tab_end:\n");
    wrt("\n\n");

#   undef D           // so' para jogar pelo seguro...
#   undef S
}


static void write_load_DLLs()
{
    TDLL  *dl;

    if (!lst_dlls)  return;

    wrt("%o%s\n", "ld", "bc, _prog_end");
    wrt("%o%s\n", "ld", "de, _dll_addr_tab");

    for (dl = lst_dlls; dl; dl = dl->prox)  {
        wrt("%o%s%u\n", "ld", "hl, ST", dl->nome);
        wrt("%o%s\n", "push", "hl");
        wrt("%o%s\n", "call", "Load_DLL");
        wrt("%o%s\n", "pop", "hl");
        wrt("%o%s\n", "jr", "c, _DLL_missing      ; exit with message if DLL not present");
    }
    wrt("\n");
}


static void write_dll_related_vars ()
{
    TDLL  *dl;

    if (!lst_dlls)  return;
    wrt("_dll_addr_tab: .dw  ");
    for (dl = lst_dlls; dl; dl = dl->prox)  {
        wrt("0");
        if (dl->prox)
           wrt(",");
    }
    wrt("\n");
}


static void write_header ()
{
    FILE  *fi = fopen ("incs.tis", "rt");
    FILE  *fe = fopen ("extra.tis", "rt");

    wrt("; Generated by TISCO - TI Simple C Compiler\n; By NSJ aka Viriato 1998\n\n");
    wrt("; These defines 'fix' TI's include files to work with TASM\n"
        "#define    equ       =\n"
        "#define    EQU       =\n"
        "#define    0FCH      0FCh\n"
        "#define    0D4H      0D4h\n"
        "#define    0E8H      0E8h\n"
        "#define    0C0H      0C0h\n\n");
   // Put TI-86 include files or other stuff
    if (fi != NULL)  {
       char line[2048];
       wrt(" ;; User includes\n");
       while (fgets(line, 2048, fi))
           wrt(line);
       fclose(fi);
       wrt("\n");
    }
    else  {
       wrt("#include \"ti86asm.inc\"\n");
       wrt("#include \"ti86ops.inc\"\n");
       wrt("#include \"ti86abs.inc\"\n");
       wrt("#include \"ti86math.inc\"\n");
       wrt("#include \"ti86undc.inc\"\n\n\n");
    }

    wrt("#define calls      call\n\n");
    wrt(" ;; A better format for some instructions...\n");
    wrt(".addinstr  ADD  B     80   1 NOP 1\n");
    wrt(".addinstr  ADD  C     81   1 NOP 1\n");
    wrt(".addinstr  ADD  (HL)  86   1 NOP 1\n");
    wrt(".addinstr  ADD  (IX*) 86DD 3 ZIX 1\n");
    wrt(".addinstr  ADD  A     87   1 NOP 1\n");
    wrt(".addinstr  ADD  *     C6   2 NOP 1\n\n");

   // Put any other stuff user wants
    if (fe != NULL)  {
       char line[2048];
       wrt(" ;; User extra text\n");
       while (fgets(line, 2048, fe))
           wrt(line);
       fclose(fe);
       wrt("\n");
    }
}


static void set_regs_param ()
{
    if (!cmdl.gen_args_code)
       return;
    wrt("%o%s\n", "ld", "hl, (_asm_reg_af)");
    wrt("%o%s\n", "ld", "(_prog_end + 0), hl");
    wrt("%o%s\n", "ld", "hl, (_asm_reg_bc)");
    wrt("%o%s\n", "ld", "(_prog_end + 2), hl");
    wrt("%o%s\n", "ld", "hl, (_asm_reg_de)");
    wrt("%o%s\n", "ld", "(_prog_end + 4), hl");
    wrt("%o%s\n", "ld", "hl, (_asm_reg_hl)");
    wrt("%o%s\n", "ld", "(_prog_end + 6), hl");
}


static void set_regs_param_dll ()
{
    if (!cmdl.gen_args_code)
       return;
    wrt("\n%o%s\n", "push", "ix");
    wrt("%o%s\n", "pop", "hl");
    wrt("%o%s\n", "ld", "de, (_asm_reg_af)");
    wrt("%o%s\n", "ld", "(hl), e");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "(hl), d");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "de, (_asm_reg_bc)");
    wrt("%o%s\n", "ld", "(hl), e");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "(hl), d");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "de, (_asm_reg_de)");
    wrt("%o%s\n", "ld", "(hl), e");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "(hl), d");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "de, (_asm_reg_hl)");
    wrt("%o%s\n", "ld", "(hl), e");
    wrt("%o%s\n", "inc", "hl");
    wrt("%o%s\n", "ld", "(hl), d");
    wrt("%o%s\n", "inc", "hl");
}


void wrt_code (TROTNUM *rn)
{
    FILE  *fe = fopen ("exdata.tis", "rt");
    char  fnom[256];

    strcat( strcpy(fnom, cmdl.src_file), ".asm" );
    fout = fopen(fnom, "wt");
    if (!fout) {
       printf("%s <- ", fnom);
       ErroF(-423, "Unable to open output file for writing");
    }

    write_header();

    switch (cmdl.tipo_prog)  {
      case tpOVERLAY :
           wrt(".org _asm_exec_ram\n\n");
           set_regs_param();
           wrt("%o%s\n", "ld", "ix, _IX_stack");
           wrt("\n%o%s\n", "ld", "a, (_asm_reg_af)");
           wrt("%s\n", "  ;  a = routine_index * 2. jump to (_routine_address_table + a)");
           wrt("%o%s\n", "ld", "hl, _routine_address_table");
           wrt("%o%s\n", "ld", "e, a");
           wrt("%o%s\n", "ld", "d, 0");
           wrt("%o%s\n", "add", "hl, de");
           wrt("%o%s\n", "ld", "a, (hl)");
           wrt("%o%s\n", "inc", "hl");
           wrt("%o%s\n", "ld", "h, (hl)");
           wrt("%o%s\n", "ld", "l, a");
           wrt("%o%s\n\n", "jp", "(hl)");
           build_routine_table();
           break;
      case tpDLL :
           wrt(".org 0\n\n");
           wrt(" ; ** space occupied in mem by the DLL\n"
               "_DLL_size:    .dw    _relocation_table - _routine_address_table\n\n");
           //wrt("DLL: .org 0\n\n");
           build_routine_table();
           gen_data_relocation_labels ();
           break;
      case tpAPP :
           wrt(".org _asm_exec_ram\n\n");
           write_load_DLLs();
           if (lst_dlls)  {
               wrt("%o%s\n", "push", "bc");
               wrt("%o%s\n", "pop", "ix");
               //set_regs_param_dll();
               wrt("%o_main\n\n", "jp");
               // Escr 1 msg dizendo q falta 1 DLL
               wrt("%s\n", "_DLL_missing:");
               wrt("%o%s\n", "push", "hl");
               wrt("%o%s\n", "ld", "hl, ST_DLL_missing");
               wrt("%o%s\n", "call", "_puts");
               wrt("%o%s\n", "pop", "hl     ; get addr of DLL name (string)");
               wrt("%o%s\n", "inc", "hl");
               wrt("%o%s\n", "inc", "hl");
               wrt("%o%s\n", "call", "_puts");
               wrt("%o%s\n", "call", "_newline");
               wrt("%o%s\n", "ret", "       ; return to TI-OS");
               wrt("%s\n\n", "ST_DLL_missing: .db \"DLL Missing:\", 0");
           }
           else  {
               //set_regs_param();
              if ((cmdl.simple_prgm) && (!lst_dlls))  {
//                  wrt("%o_main\n", "jp");
              }
              else {
                  wrt("%o%s\n", "ld", "ix, _IX_stack");
                  wrt("%o_main\n", "jp");
//                 wrt("%o\n\n", "ret");
              }
           }
           break;
    }

    write_funcoes();
    write_funcoes_lib(rn);
    write_strings();
    write_gvars();
    write_dll_related_vars();

   // Put any other stuff user wants
    if (fe != NULL)  {
       char line[2048];
       wrt("\n ;; User extra text\n");
       while (fgets(line, 2048, fe))
           wrt(line);
       fclose(fe);
    }

    write_dllusing_code();

    if (cmdl.tipo_prog == tpDLL)
       build_relocation_table();

    wrt("\n\n_prog_end:\n");
    if (cmdl.tipo_prog == tpAPP)  {
        wrt("\n_IX_stack:");
       if (cmdl.reduce_init) ;
         // wrt("\n_IX_stack:");
       else  {
          //wrt("\n;.org (_prog_end + 8)\n\n  .dw 0, 0, 0, 0");
          //wrt("\n_IX_stack:   .dw  _prog_end      "
          //    "; arg to 1st called function: ptr to regs\n");
       }
    }

    wrt("\n\n.end\n");
    wrt(".end\n");

    fclose(fout);
}

