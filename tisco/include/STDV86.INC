
  /*> stdvar86.inc <*
   *
   * Date: 01/11/1998
   * Last Change: 25/11/1998
   *
   * Basic variable manipulation rotines and external declarations
   * for TI-86.
   * Needs:
   *    stdio86.inc in C file.
   *    ti86asm.inc and ti86undc.inc in assembly file.
   *
   * By NSJ aka Viriato 1998
   *
   * USE AT YOUR OWN RISK, NO WARANTIES OF ANY KIND ARE GIVEN
   */


#ifndef __stdvar86_inc__
#define __stdvar86_inc__

#include "misc.inc"


/**************************************************************************
 ** TYPES, CONSTANTS AND MACROS                                          **/

typedef struct TABS {
   uchar   hi;
   uint    lo;
}


typedef struct TVAR {
   uchar   *name;      // Name in var name format (with sign and len)
   TABS    pdata;      // ptr to data, excluding size byte/word
   uint    pos;        // Write/read pointer (byte is the granularity)
   uint    size;       // size in bytes (must be set by user if using openv)
   char    type;       // var type
}


// blkv routine modes
#define VWRITE   0           // write block
#define VREAD    1           // read block


// createv constants
#define cvEXCL         0        // Exclusive mode
#define cvOWRT         1        // Overwrite mode

#define CRTSTRING       0x472F    // Addresses of var creation routines
#define CRTPICT         0x473B
#define CRTREAL         0x46EB
#define CRTPROG         0x474F


#define VAREXISTS     0xFF
#define UNKNOWNVAR    0xFE





#define Mov10bytesFromHLToOP1      asm "      rst  20h";


/**************************************************************************
 ** EXTERNAL VARIABLES                                                   **/


/**************************************************************************
 ** EXTERNAL FUNCTIONS (ROM FUNCTIONS)                                   **/

void MEMCHK();
void MOV10TOOP1();
void CREATESTRNG();
void FINDSYM();
void delvar();
uchar GETB_AHL();
void writeb_inc_ahl();

void cp_ahl_bde();
void ld_ahl_hl();
void ld_bde_hl();
void ld_hl_ade();
void ld_hl_bde();
void inc_ptr_ade();
void ex_ahl_bde();

void SET_ABS_SRC_ADDR();
void SET_ABS_DEST_ADDR();
void SET_MM_NUM_BYTES();
void mm_ldir();
void mm_lddr();

/**************************************************************************
 ** FUNCTIONS                                                            **/

#ifndef MAXVAR
 #define MAXVAR   3        // Define it out of here if you want
#endif

TVAR   TabVars[MAXVAR];   // Data about open vars
char   iVars;     // Gets (auto-)initialized to 0
char   vocup;     // Keeps TabVars ocupation, 1 bit per TabVars
                  // position, so max MAXVAR is 8. Auto-Initialized 0.




// ******************************************
// ABS ADDRESS routines

void abs_inc (TABS *addr, uint delta)
/* Adds a positive delta to an absolute address */
{
    getv  addr;
    asm "     push   hl";    // Save addr address for later
    ld_ahl_hl();             // ld  ahl, (hl)
    asm {
     ld    e, (ix-2)         ; get delta, 16 bit value
     ld    d, (ix-1)

     add   hl, de            ; Add delta to ahl
     adc   a, 0        };
    ex_ahl_bde();
    asm "     pop   hl";
    ld_hl_bde();             // ld  (hl), bde
}


void abs_dec (TABS *addr, uint delta)
/* Subtracts a positive delta to an absolute address */
{
    getv  addr;
    asm "     push   hl";    // Save addr address for later
    ld_ahl_hl();
    asm {
     ld    e, (ix-2)
     ld    d, (ix-1)         ; de = delta

     or    a
     sbc   hl, de            ; sub delta to ahl
     sbc   a, 0        };
    ex_ahl_bde();
    asm "     pop   hl";
    ld_hl_bde();
}


bool abs_equal (TABS *a, TABS *b)
/* Returns TRUE if *a == *b else returns FALSE */
{
    getv  a;
    ld_bde_hl();        // bde = (hl)  (undoc)
    getv  b;            // (Doesn't destroy B DE)
    ld_ahl_hl();        // ahl = (hl)  (undoc)
    cp_ahl_bde();       // (undoc?)
    ifz
       return TRUE;
    return FALSE;
}


// *********************************************************************

TVAR   *vaux;       // Aux. Used to hold TabVars[vid] in functions

void load_vaux (char vid)   // Internal use
{
    vaux = &TabVars[vid];
}


bool seekv (char vid, uint position)
/* Sets the current vid variables's position. Returns FALSE if
 * position is out of range (goes beyond the end of the variable) */
{
    load_vaux(vid);
    if (position < vaux->size)  {
       vaux->pos = position;
       return TRUE;
    }
    return FALSE;
}


void get_actual_pos_2_ahl (char vid)
{
    load_vaux(vid);
    getv  vaux->pos;
    asm "     push   hl";
    gete  vaux->pdata;
    ld_ahl_hl();               // ahl = ABS addr of data
    asm {
      pop    de
      add    hl, de
      adc    a, 0      ; Now ahl = pdata + pos
    };
    // ahl has pdata + pos
}


void get_addr_and_incr (char vid)
// Returns the ABS pos of a variable in ahl and increments "vid.pos".
// This function is for stdv86 internal use.
{
    get_actual_pos_2_ahl(vid);
    // ahl has pdata + pos
    asm {
      push   hl        ; Save ahl
      push   af                    };
    gete  vaux->pos;
    asm {                // Next position
      ld     e, (hl)
      inc    hl
      ld     d, (hl)
      inc    de           ; Next position
      ld     (hl), d
      dec    hl
      ld     (hl), e

      pop    af          ; Recover ahl
      pop    hl                         };
}


uchar getcv (char vid)
/* Gets a byte from the var's actual position and increments the position */
{
    get_addr_and_incr(vid);
    return GETB_AHL();         // a = (ahl)
}


void putcv (char vid, uchar value)
/* Puts a byte into the var's actual position and increments the position */
{
    get_addr_and_incr(vid);
    asm "     ld     c, (ix-1)";    // c = value
    writeb_inc_ahl();               // (ahl) = c
}


uint  __index;      // Aux var for the blkv routines (global var is faster)

void getblkv (char vid, char *buffer, uint blk_size)
/* Gets a block from the var's actual position and increments the position */
{
    for (__index = 1; __index <= blk_size; __index++)  {
       *buffer = getcv(vid);
       buffer++;
    }
}


void putblkv (char vid, char *buffer, uint blk_size)
/* Puts a block from the var's actual position and increments the position */
{
    for (__index = 1; __index <= blk_size; __index++)  {
       putcv(vid, *buffer);
       buffer++;
    }
}


void blkv (char vid, char *buffer, uint blk_size, char mode)
/* Puts or gets a block from the var's actual position and increments
   the position. */
{
    for (__index = 1; __index <= blk_size; __index++)  {
       if (mode)
          *buffer = getcv(vid);
       else
          putcv(vid, *buffer);
       buffer++;
    }
}



void find_sym_hl ()
// Internal use
{
    Mov10bytesFromHLToOP1;            // OP1 = var name
    FINDSYM();
}


bool findv (char *name)
/* Returns TRUE if the var name exists, else FALSE. If it exists, the
 * registers returned from FINDSYM will be intact.
 * This last fact about the registers are of concern only for stdv86,
 * and should NOT be used by a portable program.                          */
{
    // Move the name into OP1 (OP1 should be SAVED!!)
    getv  name;
    find_sym_hl();
    asm "    push   af";         // Save C flag
    asm "    and    01Fh";
    asm "    ld     c, a";       // Save var type
    asm "    pop    af";
    ifc                    // if CF = 1 var doesn't exist
       return FALSE;       // (return only uses register A)
    return TRUE;
}


void delv (char *name)
/* Deletes the var name. If it doesn't exist, nothing is done. */
{
    // Move the name into OP1 (OP1 should be SAVED!!)
    getv  name;
    find_sym_hl();
    ifnc                          // If exists, del it.
       delvar();
}



char get_free_slot ()
/* Returns a free variable slot for use.
 * It does not take care of errors like not having any free slot.
  */
{
    getv   vocup;
    asm {
     ld     c, a      ; ocupation bit vector
     ld     l, 1      ; bit mask
     ld     b, 0      ; index
_gfs_loop:
     srl    l
     ld     a, c
     and    l
     inc    b
     jr     nz, _gfs_loop    ; flags still reflect "and  l"

     ld     a, c
     or     l
     ld     (_vocup), a    ; update ocupation vector
     ld     a, b           ; return index of free slot      };
}


void closev (char vid)
/* Frees variable slot from use. */
{
    getv   vocup;
    asm {
     ld     c, a
     ld     b, (ix-1)      ; b = vid
_fs_loop:      ; find bit mask for index vid
     srl    a
     djnz   _fs_loop

     xor    0FFh           ; a = 'set bit(vid), 0' ^ FFh
     and    c              ; (c = vocup)
     ld     (_vocup), a    ; update ocupation vector       };
}


char openv (char *name)
/* Opens a variable for usage. */
/* If var not found returns UNKNOWNVAR, else VID. */
{
    char   vid;

    if (findv(name))  {
       asm "     push   bc";
       asm "     push   de";
       vid = get_free_slot();
       load_vaux(vid);
       gete  vaux->pdata;
       asm "     pop    de";
       asm "     pop    bc";
       ld_hl_bde();      // Save pointer to data
       gete  vaux->type;
       asm "     ld     (hl), c";   // c has var type, returned from findv
       vaux->pos = 0;
       vaux->size = 2;   // Enough for loading the size, if exists
       return vid;
    }

    return UNKNOWNVAR;
}


void set_varsize (char vid, uchar size)
{
    TabVars[vid].size = size;
}


uint get_varsize (char vid)
{
    return TabVars[vid].size;
}


char get_vartype (char vid)
{
    return TabVars[vid].type;
}


int get_varpos (char vid)
{
    return TabVars[vid].pos;
}



char createv (char *name, uint creat_routine, uint total_size,
              uint sizei, uchar mode)
// The name of the var to be created must start
// at the third character. The second character must have
// the lenght of the name. The first one doesn't matter.
// The string name must be keept intact after the create.
// Ex.: " \005hello"
// Any sizes necessary for variable creation routines are passed in sizei.
// creat_routine has the address of the creation routine.
// mode is the creation mode, exclusive or overwrite.
// total_size id the var's total size in bytes
// This function returns the variable id (vID) or VAREXISTS.
{
    char  vid;

    if (mode == cvOWRT)
       delv(name);

    getv  name;
    find_sym_hl();
    ifnc                   // if CF = 0 var does exist
       return VAREXISTS;

    vid = get_free_slot();
    load_vaux(vid);

    vaux->size = total_size;
    vaux->pos = 0;

    // Move the name into OP1 (OP1 should be SAVED!!)
    getv  name;
    Mov10bytesFromHLToOP1;

    getv  creat_routine;
    asm "     ld     (_cv_ccall+1), hl";    // Alters next call's addr
    getv  sizei;

    asm "_cv_ccall:";
    asm "     call   0";     // Call var creation routine

    asm "     push   de";
    gete  vaux->pdata;       // (preserves bc)
    asm "     pop    de";
    ld_hl_bde();             // pdata = bde = ABS addr of var data
    gete  vaux->type;        // (preserves bc)
    asm "     ld     a, (_OP4)";  // get var type
    asm "     and    01Fh";
    asm "     ld     (hl), a";

    return vid;
}


void get_var_pos_2_ahl (char vid, int pos)    // Internal use
{
    load_vaux(vid);
    getv  pos;
    asm "     push   hl";
    gete  vaux->pdata;
    ld_ahl_hl();               // ahl = ABS addr of data
    asm {
      pop    de
      add    hl, de
      adc    a, 0      ; Now ahl = pdata + pos
    };
    // ahl has pdata + pos
}


void copyv (char vid_to, uint pos_to, char vid_from, uint pos_from, uint size)
/* Copies a block of bytes from one var to another */
{
    get_var_pos_2_ahl(vid_from, pos_from);
    SET_ABS_SRC_ADDR();

    get_var_pos_2_ahl(vid_to, pos_to);
    SET_ABS_DEST_ADDR();

    getv  size;
    asm "     xor   a";
    SET_MM_NUM_BYTES();

    mm_ldir();
}


void copyvb (char vid_to, uint pos_to, char vid_from, uint pos_from, uint size)
/* Copies a block of bytes from one var to another */
{
    get_var_pos_2_ahl(vid_from, pos_from+size);
    SET_ABS_SRC_ADDR();

    get_var_pos_2_ahl(vid_to, pos_to+size);
    SET_ABS_DEST_ADDR();

    getv  size;
    asm "     xor   a";
    SET_MM_NUM_BYTES();

    mm_lddr();
}



void varset (char vid, uint pos, uint size, char value)
/* Fills a var block with a value */
{
    seekv(vid, pos);
    for (; size; size--)
        putcv(vid, value);
}




#endif

