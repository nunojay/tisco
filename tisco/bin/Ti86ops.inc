;
;
;       Support routines for OP1 - OP6 registers
;
;
;
_OP3TOOP4       equ            4203h            ; op3 -> op4, 11 bytes moved
_OP1TOOP4       equ            4207h
_OP2TOOP4       equ            420Bh
_OP4TOOP2       equ            4213h
_OP4TOOP3       equ            4217h
_OP3TOOP2       equ            421Bh
_OP1TOOP3       equ            421Fh
_OP5TOOP2       equ            4227h
_OP5TOOP6       equ            422Bh
_OP5TOOP4       equ            422Fh
_OP1TOOP2       equ            4233h
_OP6TOOP2       equ            423Bh
_OP6TOOP1       equ            423Fh
_OP4TOOP1       equ            4243h
_OP5TOOP1       equ            4247h
_OP3TOOP1       equ            424Bh
_OP4TOOP5       equ            424Fh
_OP3TOOP5       equ            4253h
_OP2TOOP5       equ            4257h
_OP2TOOP6       equ            425Fh
_OP1TOOP6       equ            4263h
_OP1TOOP5       equ            426Bh
_OP2TOOP1       equ            426Fh
;
_MOV10B         equ            427Bh            ; move 10 bytes
_MOV18          equ            427Fh
_mov9b          equ            4283h
_MOV8B          equ            4287h
_MOV7B          equ            428Bh
_MOV6B          equ            4293h
_MOV5B          equ            4297h
_MOV4B          equ            429Bh
_MOV3B          equ            429Fh
_MOV2B          equ            42A3h
_OP2TOOP3       equ            42CBh
_OP4TOOP6       equ            42D3h
_MOV10TOOP1     equ            42D7h     ; 10 bytes @ (hl) -> op1 non banked
_MOV10TOOP2     equ            42DFh     ; 10 bytes @ (hl) -> op2 non banked
_MOVFROP1       equ            42E3h     ; 10 bytes @ op1 -> (de) non banked
;
_OP4SET1        equ            42E7h     ; op4 = floatin point 1
_OP3SET1        equ            42EBh
_OP2SET8        equ            42EFh
_OP2SET5        equ            42F7h
_OP2SET4        equ            42FBh
_OP2SET3        equ            4303h
_op1set1        equ            430Fh
_OP1SET4        equ            4313h
_OP1SET3        equ            4317h
_OP3SET2        equ            431Bh
_OP1SET2        equ            431Fh
_OP2SET2        equ            4323h
_OP2SET1        equ            432Fh
_OP4SET0        equ            4353h
_OP3SET0        equ            4357h
_OP2SET0        equ            435Bh
_OP1SET0        equ            435Fh
;
_ZEROOP1M       equ            4367h            ; mantissa op1 = 0's
_ZEROOP1        equ            436Bh            ; 11 bytes op1 = 0
_ZEROOP2        equ            436Fh
_ZEROOP3        equ            4373h
;
_CPOP1OP2       equ            41FBh

_SetXXOP1       equ            4613h
_SetXXXXOP2     equ            461Bh
_SetXXOP2       equ            4617h
