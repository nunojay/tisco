
_winTop             equ         0D13Dh	;first homescreen row
_winBtm             equ         0D13Eh	;last homescreen row
_winLeftEdge        equ         0D13Fh
_winLeft            equ         0D140h
_winAbove           equ         0D142h
_winRow             equ         0D144h
_winCol             equ         0D146h

_clrText            equ         4AB5h   ;(set up text shadow & copy to cmd shadow)
;$4AB9   $3AD7   _setText (set up text shadow)
;$4ABD   $3AD1   _setText_stay - set up text shadow ($28f8, see above)
;$4AC0   $3ADD   _clrText_stay - copy text shadow to screen ($28f8)
_cpyText            equ         4AC3h   ; (copy text shadow->screen)


;
;   TI-86 undocumented ROM calls used by TISCO includes
;

_dispAHL         equ    4A33h
_dispOP1         equ    515Bh

_calcOff         equ    4101h
_pause           equ    4D43h
_homeup          equ    4A95h

_cursorOn        equ    4994h
_cursorOff       equ    498Ch
__SetUpperAlpha   equ    407Ch    ; set upper case alpha lock
__ClearUpperAlpha equ    4074h    ; clear upper case alphalock
_enough_mem      equ    46DBh    ; hl = mem needed; CF = 1 if not enough mem

_ldhlind         equ    4010h    ; hl = (hl)

_inc_ptr_ade     equ    45EFh    ; ade++
_ex_ahl_bde      equ    45F3h    ; ex   ahl, bde
_ld_ade_hl       equ    45F7h    ; ade = (hl), hl += 3
_ld_bde_hl       equ    45FBh    ; bde = (hl), hl += 3
_ld_ahl_hl       equ    45FFh    ; ahl = (hl)
_ld_hl_ade       equ    4603h    ; (hl) = ade, hl += 3
_ld_hl_bde       equ    4607h    ; (hl) = bde, hl += 3
_get_word_ahl    equ    521Dh    ; de = (ahl), ahl += 2
_set_word_ahl    equ    521Dh    ; (ahl) = de, ahl += 2
_add_ahl_2       equ    4C3Fh    ; ahl += 2
_cp_ahl_bde      equ    46C7h    ; Compares ahl and bde and sets Z flag

_ckop1pos        equ    450Fh    ; OP1 >= 0  =>  ZF = 1
_ckop1fp0        equ    449Bh    ; OP1 == 0  =>  ZF = 1
_negOP1          equ    5494h    ; OP1 = -OP1
_pushOP1         equ    4813h
_popOP1          equ    47B7h
_abs             equ    4517h    ; a = sign(OP1); OP1 = abs(OP1)

_HLdivA          equ    4044h

__strlen         equ    4957h    ; bc = strlen(hl). Preserves all.
__strcpy         equ    495Bh    ; strcpy(de, hl). Destroys A.
__strcat         equ    495Fh    ; strcat(de, hl). Destroys A.
__strcmp         equ    4963h    ; strcmp(hl, de), strings with length bytes.
                                 ; Destroys A.



