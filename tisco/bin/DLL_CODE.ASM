
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;; By Nuno João  (Nuno Jo&atilde;o)
 ;; nupsnover@gmail.com
 ;; Use this code as you please, but give me some credits...
 ;; Given without any warranty of any kind.

 ;; Here is the 'help' code for using DLL's, used by TISCO v0.0
 ;; Back_Patch is TI-8x independent, but Load_DLL is TI-86 code only.
 ;; Feel free to improve this code, I encourage you. Oh, and send me
 ;; a copy...


 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;; Back-patch a call to a DLL routine

 ;; Only the flags are destroyed, so we can call a DLL routine
 ;; passing it values in registers, but not on the flags (only the
 ;; first time, times after that the DLL routine will be called
 ;; directly, so no problem).
 ;; Usage:
 ;;                call   routine_stub
 ;;                ...
 ;;
 ;;  routine_stub: call   Back_Patch
 ;;                .db    DLL_number * 2, routine_index * 2
 ;;

 ;;  Input: TOS has the addr of a memory location with 2 bytes:
 ;;             DLL_number * 2
 ;;             routine_index * 2
 ;;         TOS + 2 has the caller's return addr
 ;; Output: the caller who called the routine who called this routine
 ;;         will see it's call instruction altered, with the real DLL
 ;;         routine addr instead of the addr of the stub.
 ;;         The TOS is poped (eliminates the call to Back_Patch).

 ;; This routine has 55 bytes, and demands 5 bytes per stub (or per
 ;; DLL routine used).
 ;; This routine execs in 359 cycles.

hl_save: .dw 0
de_save: .dw 0

Back_Patch:
    ld     (de_save), de   ; save these registers so that they pass to
    ld     (hl_save), hl   ; the called routine

    pop    hl              ; get addr of doubled DLL number & function number
    push   bc              ; save bc

    ld     c, (hl)
    inc    hl
    ld     e, (hl)

    ld     hl, _dll_addr_tab
    ld     b, 0
    ld     d, b
    add    hl, bc          ; hl = d + _dll_addr_tab
    ld     b, (hl)
    inc    hl
    ld     h, (hl)
    ld     l, b            ; hl = DLL start addr

    pop    bc              ; restore bc, we don't need it any more

    push   hl
    inc    hl
    inc    hl              ; Now hl = addr of DLL's routine table

    add    hl, de          ; de has function_index * 2
    ld     e, (hl)
    inc    hl
    ld     d, (hl)         ; de = addr of function relative to DLL
    pop    hl              ; get DLL start addr
    add    hl, de
    ex     de, hl          ; Now de = function's addr

    ex     (sp), hl        ; get caller's return addr from TOS
    dec    hl              ; by decrementing it we get a pointer to the
                           ; addr of the call instruction hi byte addr
    ld     (hl), d
    dec    hl
    ld     (hl), e         ; back patch - change the caller's addr (in the
                           ; code) to the DLL routine's addr
    inc    hl
    inc    hl
    ex     (sp), hl        ; restore the return addr to the TOS

  ; at the end of Back_Patch, jp to the DLL routine (with ret)
    push   de

    ld     de, (de_save)
    ld     hl, (hl_save)

    ret                    ; jump to the DLL's routine



 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;; Load a DLL into memory, relocating DLL's code.
 ;; TI-86 code

 ;;  Input: hl = ptr to DLL's name (var name),
 ;;         de = ptr to DLL-start-addr store location
 ;;         bc = addr of first byte of free mem (at the end of the program)
 ;; Output: de = ptr to DLL-start-addr next store location
 ;;         bc = addr of new first byte of free mem (after the DLL)
 ;;         CF = 1 iif DLL does not exist
 ;;
 ;; If you look well, you will see that this function was made to be
 ;; called easelly several consecutive times, changing only hl.

 ;; This routine has 105 bytes, execs in a minimum of 630 cycles

 ;; Var (program var) DLL format:
 ;;    0    w         size of program (set by TI-OS)
 ;;    2    w         asm program token
 ;;    4    w         size of DLL (RAT + code + global data)
 ;;    6    w units   Routine address table (relative to DLL), RAT.
 ;;    8    w         DLL data (code; data)
 ;;   8+x   w units   item relocation table, ends with a 0 word.

 ;; All DLL code is relative to 'size of DLL'.

_this_dll_addr: .dw   0
_ptr_dll_saddr: .dw   0

Load_DLL:
    ld     (_ptr_dll_saddr), de
    ld     (_this_dll_addr), bc

    rst    20h
    call   _FINDSYM
    ret    c                  ; return if DLL var does not exist
    call   _ex_ahl_bde

    call   _get_word_ahl      ; jump over the var's size(?)
    call   _get_word_ahl      ; jump over the asm program token
    call   _get_word_ahl      ; de = size of DLL, ahl points to DLL RAT
    push   de                 ; the size will be needed twice, later
    push   de

   ; copy DLL to exec assembly memory, rigth after the program
    call   _SET_ABS_SRC_ADDR
    xor    a
    ld     hl, (_this_dll_addr)
    inc    hl
    inc    hl                 ; leave space for _DLL_size
    call   _SET_ABS_DEST_ADDR
    xor    a
    pop    hl                 ; size of DLL
    call   _SET_MM_NUM_BYTES

    call   _mm_ldir

   ; _ABS_SRC_ADDR is used from now on to keep a pointer to the relocation
   ; table. It was incremented by _mm_ldir and now points to the beggining
   ; of the RT, wich follows the DLL data

next_item:

    ld     a, (_ABS_SRC_ADDR)
    ld     hl, (_ABS_SRC_ADDR+1)

    call   _get_word_ahl       ; de = (ahl), ahl += 2
                               ; de = addr (relative to the start of the
                               ;    DLL) of item to relocate
    ld     b, a                ; save a
    ld     a, d
    or     e
    jr     z, no_more_items    ; exit if end of table
    ld     a, b

    call   _SET_ABS_SRC_ADDR   ; ahl -> (_ABS_SRC_ADDR)

    ex     de, hl
    ld     de, (_this_dll_addr)
    add    hl, de              ; hl = addr of item to relocate
    ld     a, (hl)
    add    a, e
    ld     (hl), a
    inc    hl
    ld     a, (hl)
    adc    a, d
    ld     (hl), a            ; (addr_item_to_relocate) += _this_dll_addr

    jr     next_item

no_more_items:

    pop    de            ; de = size of DLL
    ld     hl, (_this_dll_addr)
    push   hl
    add    hl, de
    push   hl
    pop    bc            ; bc = addr of next DLL -2 (this_dll_addr + dll_size)
    inc    bc
    inc    bc            ; account for _DLL_size wich is not included in
                         ; _DLL_size

    ld     hl, (_ptr_dll_saddr)
    pop    de            ; de = (_this_dll_addr)
    ld     (hl), e
    inc    hl
    ld     (hl), d       ; ((_ptr_dll_saddr)) = _this_dll_addr

    inc    hl
    ex     de, hl        ; de = ptr to location to store next DLL addr

    or     a             ; CF = 0

    ret


