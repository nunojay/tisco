;
;
;       TI-86 'Paged Ram' Support routines
;
;

;
;       some ram equates
;
_ABS_SRC_ADDR      equ         0C392h   ; 3 byte ABS address pointer
_ABS_DEST_ADDR     equ         0C395h   ; 3 byte ABS address pointer
_MM_NUM_BYTES      equ         0C398h   ; 3 byte block move counter
;

;
;
;       'Block' Move Support Routines
;
;

;
; ahl -> (_abs_src_addr)
;
_SET_ABS_SRC_ADDR                equ            4647h

;
; ahl -> (_abs_dest_addr)
;
_SET_ABS_DEST_ADDR               equ            5285h

;
; ahl -> (_mm_num_bytes)
;
_SET_MM_NUM_BYTES                equ            464Fh


;
;       This routine mimmicks the Z80's LDIR instruction.
;
;
; INPUTS       - abs_src_addr, abs_dest_addr, mm_num_bytes
;                Source Address, Destination Address, and # of bytes,
;                all in 24-bit form.
; OUTPUTS      - abs_src_addr, abs_dest_addr, mm_num_bytes
;                Modified exactly as HL, DE, and BC are after LDIR
;
;  moves (_mm_num_bytes) bytes starting @ (_abs_src_addr) to (_abs_dest_addr)
;  incrementing the src/dest pointer after each byte moved
;
_mm_ldir           equ         52EDh

;
;
;       This routine mimmicks the Z80's LDDR instruction.
;
;
; INPUTS       - abs_src_addr, abs_dest_addr, mm_num_bytes
;                Source Address, Destination Address, and # of bytes,
;                all in 24-bit form.
; OUTPUTS      - abs_src_addr, abs_dest_addr, mm_num_bytes
;                Modified exactly as HL, DE, and BC are after LDIR
;
;  moves (_mm_num_bytes) bytes starting @ (_abs_src_addr) to (_abs_dest_addr)
;  decrementing the src/dest pointer after each byte moved
;
_mm_lddr           equ         52F1h
;

;
;  ahl -> (_abs_src_addr). move 10 bytes @ (_abs_src_addr) -> OP1
;
_ABS_MOV10TOOP1    equ         5235h

;
;  move 10 bytes @ (_abs_src_addr) -> OP1
;
_ABS_MOV10TOOP1_noset   equ    5239h

;
; ahl -> (_abs_dest_addr).move 10 bytes from (_abs_src_addr)->(_abs_dest_addr)
;
_ABS_MOV10B_SET_D  equ         523Dh

;
; move 10 bytes from (_abs_src_addr) -> (_abs_dest_addr)
;
_ABS_MOV10B        equ         5249h

;
; ahl -> (_abs_dest_addr). move 10 bytes from OP1 -> (_abs_dest_addr)
;
_ABS_MOVFROP1_SET_D    equ     5241h

;
; move 10 bytes from OP1 -> (_abs_dest_addr)
;
_ABS_MOVFROP1      equ         5245h

;
;
;       Misc support
;
;
_inc_ptr_ahl    equ    4637h    ; ahl = ahl + 1
_dec_ptr_ahl    equ    463Bh    ; ahl = ahl - 1
_inc_ptr_bde    equ    463Fh    ; bde = bde + 1 , ahl & status intact
_dec_ptr_dbe    equ    4643h    ; bde = bde - 1 , ahl & status intact
_dec_ptr_ade    equ    46BFh    ; ade = ade - 1


;
; decode ABS ahl and load the RAM page and set hl = offset, (hl) = (ABS ahl)
;
_load_ram_ahl   equ    462Fh

_GETB_AHL       equ    46C3h ; acc = (ABS ahl), hl = asic address now


_RAM_PAGE_1     equ    47E3h ; load RAM page 1, FPS page
_RAM_PAGE_7     equ    47F3h ; load RAM page 7, symbol table page

;
;  write c -> (ABS ahl), ahl = ahl + 1, still ABS address
;
_writeb_inc_ahl    equ     5567h

