;
;       version : 10/20/97
;
;
;       some helpful system routine addresses
;
;
;       For now refer to existing TI-85 or TI-83 documentation
;       from the WEB.
;
;
_clrLCD                          equ            4A7Eh
_clrScrn                         equ            4A82h
_putmap                          equ            4A27h
_putc                            equ            4A2Bh
_puts                            equ            4A37h
_putps                           equ            4A3Bh
_vputmap                         equ            4AA1h
_vputs                           equ            4AA5h
_vputsn                          equ            4AA9h
_GRBUFCPY                        equ            4E39h
_ILine                           equ            4E51h
_IPoint                          equ            4E59h
_PDspGrph                        equ            4D6Fh
_CLine                           equ            51E9h
_getkey                          equ            55AAh
_getky                           equ            5371h
_RANDOM                          equ            55DAh
_newline                         equ            4A5Fh
_runindicoff                     equ            4AB1h
_runindicon                      equ            4AADh
_CONVOP1                         equ            5577h
;
;       VARIABLE ROUTINES SEE FILE 'VARS.DOC' & '
;
_RCLY                            equ            4CDFh   ; rcl y to OP1
_RCLX                            equ            4CE3h   ; rcl x to OP1
;
_CREATEREAL    equ      46EBh   ; CREATE REAL VAR OP1, NO INPUTS
;
_CREATECPLX    equ      46F7h   ; CREATE CPLX VAR OP1, NO INPUTS
;
_CREATERCONST  equ      46EFh   ; CREATE REAL CONSTANT VAR OP1, NO INPUTS
_CREATECCONST  equ      46F3h   ; CREATE CPLX CONSTANT VAR OP1, NO INPUTS
;
_CREATERVECT   equ      46FFh   ; CREATE REAL VECTOR OP1, L = # ELEMENTS
_CREATECVECT   equ      4707h   ; CREATE CPLX VECTOR OP1, L = # ELEMENTS
;
_CREATERLIST   equ      470Fh   ; CREATE REAL LIST OP1, HL = # ELEMENTS
_CREATECLIST   equ      4717h   ; CREATE CPLX LIST OP1, HL = # ELEMENTS
;
_CREATERMAT    equ      471Fh   ; CREATE REAL MATRIX OP1, HL = ROW,COL
_CREATECMAT    equ      4727h   ; CREATE CPLX MATRIX OP1, HL = ROW,COL
;
_CREATESTRNG   equ      472Fh   ; CREATE STRING OP1, HL = # BYTES
;
_CREATEEQU     equ      4737h   ; CREATE EQUATION OP1, HL = # BYTES
;
_CREATEPICT    equ      473Bh   ; CREATE PICTURE VAR OP1, NO INPUTS
;
_CREATEPROG    equ      474Fh   ; CREATE PROGRAM VAR OP1, HL = # BYTES
;
_FINDSYM       equ      46CBh   ; LOOK UP VAR
_delvar        equ      475Fh   ; DELETE VAR
;
;  ASM MODULE EXECUTOR
;
_exec_assembly equ      5730h
;
;  RAM EQUATES FOR ASM MODULE CALLING
;
_asm_reg_af    equ         0D624h
_asm_reg_a     equ         0D625h
_asm_reg_l     equ         0D626h
_asm_reg_hl    equ         0D626h
_asm_reg_h     equ         0D627h
_asm_reg_bc    equ         0D628h
_asm_reg_c     equ         0D628h
_asm_reg_b     equ         0D629h
_asm_reg_de    equ         0D62Ah
_asm_reg_e     equ         0D62Ah
_asm_reg_d     equ         0D62Bh
;
;
;       start address for all ASM programs
;
;
_asm_exec_ram             equ         0D748h
;
;
;       op1 to op6 ram equates
;
;
_OP1                      equ         0C089h
_OP1EXPM                  equ         0C08Ah
_OP1EXPL                  equ         0C08Bh
_OP1M                     equ         0C08Ch
_OP1EXT                   equ         0C093h
_OP2                      equ         0C094h
_OP2EXPM                  equ         0C095h
_OP2EXPL                  equ         0C096h
_OP2M                     equ         0C097h
_OP2EXT                   equ         0C09Eh
_OP3                      equ         0C09Fh
_OP3EXPM                  equ         0C0A0h
_OP3EXPL                  equ         0C0A1h
_OP3M                     equ         0C0A2h
_OP3EXT                   equ         0C0A9h
_OP4                      equ         0C0AAh
_OP4EXPM                  equ         0C0ABh
_OP4EXPL                  equ         0C0ACh
_OP4M                     equ         0C0ADh
_OP4EXT                   equ         0C0B4h
_OP5                      equ         0C0B5h
_OP5EXPM                  equ         0C0B6h
_OP5EXPL                  equ         0C0B7h
_OP5M                     equ         0C0B8h
_OP5EXT                   equ         0C0BFh
_OP6                      equ         0C0C0h
_OP6EXPM                  equ         0C0C1h
_OP6EXPL                  equ         0C0C2h
_OP6M                     equ         0C0C3h
_OP6EXT                   equ         0C0CAh
;
;
;       screen shadow ram equates
;
;
_textShadow               equ         0C0F9h
_cmdShadow                equ         0CFABh
_plotSScreen              equ         0C9FAh
;
;
;       display cursor ram equates
;
;
_curRow                   equ         0C00Fh
_curCol                   equ         0C010h
_penCol                   equ         0C37Ch
_penRow                   equ         0C37Dh
;
;
;       link port equates
;
;
BPort           equ     7               ;4-bit I/O
D0LD1L	    EQU 	     0FCH
D0LD1H	    EQU 	     0D4H
D0HD1L	    EQU 	     0E8H
D0HD1H	    EQU 	     0C0H
;
;       SYSTEM FLAG EQUATES
;       ALL OF THESE FLAGS CAN BE ACCESSED THROUGH THE 'IY' REGISTER
;
trigflags       EQU       0           ; IY OFFSET VALUE
trigdeg         EQU       2
;
plotflags       EQU       2           ; IY OFFSET VALUE
plotloc         EQU       1
plotdisp        EQU       2
;
grfmodeflags    EQU       2           ; IY OFFSET VALUE
grffuncm        EQU       4
grfpolarm       EQU       5
grfparamm       EQU       6
grfrecurm       EQU       7
;
graphflags      EQU       3           ; IY OFFSET VALUE
graphdraw       EQU       0
graphcursor     EQU       2
;
grfdbflags      EQU       4           ; IY OFFSET VALUE
grfdot          EQU       0
grfsimul        EQU       1
grfgrid         EQU       2
grfpolar        EQU       3
grfnocoord      EQU       4
grfnoaxis       EQU       5
grflabel        EQU       6
;
textflags       EQU       5           ; IY OFFSET VALUE
textEraseBelow  EQU       1
textScrolled    EQU       2
textInverse     EQU       3
;
onflags         EQU       9           ; IY OFFSET VALUE
onRunning       EQU       3
onInterrupt     EQU       4
;
fmtflags        EQU       10          ; IY OFFSET VALUE
fmtExponent     EQU       0
fmtEng          EQU       1
;
curflags        EQU       12          ; IY OFFSET VALUE
curAble         EQU       2
curOn           EQU       3
curLock         EQU       4
;
appflags        EQU       13          ; IY OFFSET VALUE
appTextSave     EQU       1
appAutoScroll   EQU       2
;
indicflags      EQU       18          ; IY OFFSET VALUE
indicRun        EQU       0
indicOnly       EQU       2
;
shiftflags      EQU       18          ; IY OFFSET VALUE
shift2nd        equ       3
shiftAlpha      equ       4
shiftLwrAlph    equ       5
shiftALock      equ       6
;
new_grf_flgs    EQU       24          ; IY OFFSET VALUE
textwrite       EQU       6
;
;
;    character font equates
;
;
Lbinary	equ	001h
Loctal	equ	002h
Ldecimal	equ	003h
Lhexadecimal	equ	004h
Lconvert	equ	005h
LsqUp	equ	006h
LsqDown	equ	007h
Lintegral	equ	008h
Lcross	equ	009h
LhexA	equ	00Ah
LhexB	equ	00Bh
LhexC	equ	00Ch
LhexD	equ	00Dh
LhexE	equ	00Eh
LhexF	equ	00Fh
Lroot	equ	010h
Linverse	equ	011h
Lsquare	equ	012h
Langle	equ	013h
Ldegree	equ	014h
Lradian	equ	015h
Ltranspose	equ	016h
LLE	equ	017h
LNE	equ	018h
LGE	equ	019h
Lneg	equ	01Ah
Lexponent	equ	01Bh
Lstore	equ	01Ch
Lten	equ	01Dh
LupArrow	equ	01Eh
LdownArrow	equ	01Fh
Lspace	equ	020h
Lexclam	equ	021h
Lquote	equ	022h
Lpound	equ	023h
Ldollar	equ	024h
Lpercent	equ	025h
Lampersand	equ	026h
Lapostrophe	equ	027h
LlParen	equ	028h
LrParen	equ	029h
Lasterisk	equ	02Ah
LplusSign	equ	02Bh
Lcomma	equ	02Ch
Ldash	equ	02Dh
Lperiod	equ	02Eh
Lslash	equ	02Fh
L0	equ	030h
L1	equ	031h
L2	equ	032h
L3	equ	033h
L4	equ	034h
L5	equ	035h
L6	equ	036h
L7	equ	037h
L8	equ	038h
L9	equ	039h
Lcolon	equ	03Ah
Lsemicolon	equ	03Bh
LLT	equ	03Ch
LEQ	equ	03Dh
LGT	equ	03Eh
Lquestion	equ	03Fh
LatSign	equ	040h
LcapA	equ	041h
LcapB	equ	042h
LcapC	equ	043h
LcapD	equ	044h
LcapE	equ	045h
LcapF	equ	046h
LcapG	equ	047h
LcapH	equ	048h
LcapI	equ	049h
LcapJ	equ	04Ah
LcapK	equ	04Bh
LcapL	equ	04Ch
LcapM	equ	04Dh
LcapN	equ	04Eh
LcapO	equ	04Fh
LcapP	equ	050h
LcapQ	equ	051h
LcapR	equ	052h
LcapS	equ	053h
LcapT	equ	054h
LcapU	equ	055h
LcapV	equ	056h
LcapW	equ	057h
LcapX	equ	058h
LcapY	equ	059h
LcapZ	equ	05Ah
LlBrack	equ	05Bh
Lbackslash	equ	05Ch
LrBrack	equ	05Dh
Lcaret	equ	05Eh
Lunderscore	equ	05Fh
Lbackquote	equ	060h
La	equ	061h
Lb	equ	062h
Lc	equ	063h
Ld	equ	064h
Le	equ	065h
Lf	equ	066h
Lg	equ	067h
Lh	equ	068h
Li	equ	069h
Lj	equ	06Ah
Lk	equ	06Bh
Ll	equ	06Ch
Lm	equ	06Dh
Ln	equ	06Eh
Lo	equ	06Fh
Lp	equ	070h
Lq	equ	071h
Lr	equ	072h
Ls	equ	073h
Lt	equ	074h
Lu	equ	075h
Lv	equ	076h
Lw	equ	077h
Lx	equ	078h
Ly	equ	079h
Lz	equ	07Ah
LlBrace	equ	07Bh
Lbar	equ	07Ch
LrBrace	equ	07Dh
Ltilde	equ	07Eh
LinvEQ	equ	07Fh
Lsub0	equ	080h
Lsub1	equ	081h
Lsub2	equ	082h
Lsub3	equ	083h
Lsub4	equ	084h
Lsub5	equ	085h
Lsub6	equ	086h
Lsub7	equ	087h
Lsub8	equ	088h
Lsub9	equ	089h
LcapAAcute	equ	08Ah
LcapAGrave	equ	08Bh
LcapACaret	equ	08Ch
LcapADier	equ	08Dh
LaAcute	equ	08Eh
LaGrave	equ	08Fh
LaCaret	equ	090h
LaDier	equ	091h
LcapEAcute	equ	092h
LcapEGrave	equ	093h
LcapECaret	equ	094h
LcapEDier	equ	095h
LeAcute	equ	096h
LeGrave	equ	097h
LeCaret	equ	098h
LeDier	equ	099h
LcapIAcute	equ	09Ah
LcapIGrave	equ	09Bh
LcapICaret	equ	09Ch
LcapIDier	equ	09Dh
LiAcute	equ	09Eh
LiGrave	equ	09Fh
LiCaret	equ	0A0h
LiDier	equ	0A1h
LcapOAcute	equ	0A2h
LcapOGrave	equ	0A3h
LcapOCaret	equ	0A4h
LcapODier	equ	0A5h
LoAcute	equ	0A6h
LoGrave	equ	0A7h
LoCaret	equ	0A8h
LoDier	equ	0A9h
LcapUAcute	equ	0AAh
LcapUGrave	equ	0ABh
LcapUCaret	equ	0ACh
LcapUDier	equ	0ADh
LuAcute	equ	0AEh
LuGrave	equ	0AFh
LuCaret	equ	0B0h
LuDier	equ	0B1h
LcapCCed	equ	0B2h
LcCed	equ	0B3h
LcapNTilde	equ	0B4h
LnTilde	equ	0B5h
Laccent	equ	0B6h
Lgrave	equ	0B7h
Ldieresis	equ	0B8h
LquesDown	equ	0B9h
LexclamDown	equ	0BAh
Lalpha	equ	0BBh
Lbeta	equ	0BCh
Lgamma	equ	0BDh
LcapDelta	equ	0BEh
Ldelta	equ	0BFh
Lepsilon	equ	0C0h
Ltheta	equ	0C1h
Llambda	equ	0C2h
Lmu	equ	0C3h
Lpi	equ	0C4h
Lrho	equ	0C5h
LcapSigma	equ	0C6h
Lsigma	equ	0C7h
Ltau	equ	0C8h
Lphi	equ	0C9h
LcapOmega	equ	0CAh
LxMean	equ	0CBh
LyMean	equ	0CCh
LsupX	equ	0CDh
Lellipsis	equ	0CEh
Lleft	equ	0CFh
Lblock	equ	0D0h
Lper	equ	0D1h
Lhyphen	equ	0D2h
Larea	equ	0D3h
Ltemp	equ	0D4h
Lcube	equ	0D5h
Lenter	equ	0D6h
LblockArrow     equ     0D7h
;
;
;       'getkey' equates
;
;
kRight	equ	001h
kLeft	equ	002h
kUp	equ	003h
kDown	equ	004h
kColon	equ	005h
kEnter	equ	006h
kExit	equ	007h
kClear	equ	008h
kDel	equ	009h
kIns	equ	00Ah
kNext	equ	00Bh
kAdd	equ	00Ch
kSub	equ	00Dh
kMul	equ	00Eh
kDiv	equ	00Fh
kExpon	equ	010h
kLParen	equ	011h
kRParen	equ	012h
kLBrack	equ	013h
kRBrack	equ	014h
kEqual	equ	015h
kStore	equ	016h
kRecall	equ	017h
kComma	equ	018h
kAng	equ	019h
kChs	equ	01Ah
kDecPnt	equ	01Bh
k0	equ	01Ch
k1	equ	01Dh
k2	equ	01Eh
k3	equ	01Fh
k4	equ	020h
k5	equ	021h
k6	equ	022h
k7	equ	023h
k8	equ	024h
k9	equ	025h
kEE	equ	026h
kSpace	equ	027h
kCapA	equ	028h
kCapB	equ	029h
kCapC	equ	02Ah
kCapD	equ	02Bh
kCapE	equ	02Ch
kCapF	equ	02Dh
kCapG	equ	02Eh
kCapH	equ	02Fh
kCapI	equ	030h
kCapJ	equ	031h
kCapK	equ	032h
kCapL	equ	033h
kCapM	equ	034h
kCapN	equ	035h
kCapO	equ	036h
kCapP	equ	037h
kCapQ	equ	038h
kCapR	equ	039h
kCapS	equ	03Ah
kCapT	equ	03Bh
kCapU	equ	03Ch
kCapV	equ	03Dh
kCapW	equ	03Eh
kCapX	equ	03Fh
kCapY	equ	040h
kCapZ	equ	041h
ka	equ	042h
kb	equ	043h
kc	equ	044h
kd	equ	045h
ke	equ	046h
kf	equ	047h
kg	equ	048h
kh	equ	049h
ki	equ	04Ah
kj	equ	04Bh
kk	equ	04Ch
kl	equ	04Dh
km	equ	04Eh
kn	equ	04Fh
ko	equ	050h
kp	equ	051h
kq	equ	052h
kr	equ	053h
ks	equ	054h
kt	equ	055h
ku	equ	056h
kv	equ	057h
kw	equ	058h
kx	equ	059h
ky	equ	05Ah
kz	equ	05Bh
kVarx	equ	05Ch
kAns	equ	05Dh
kPi	equ	05Eh
kInv	equ	05Fh
kSin	equ	060h
kASin	equ	061h
kCos	equ	062h
kACos	equ	063h
kTan	equ	064h
kATan	equ	065h
kSquare	equ	066h
kSqrt	equ	067h
kLn	equ	068h
kExp	equ	069h
kLog	equ	06Ah
kALog	equ	06Bh
kMath	equ	06Ch
kCplx	equ	06Dh
kString	equ	06Eh
kTest	equ	06Fh
kConv	equ	070h
kChar	equ	071h
kBase	equ	072h
kCustom	equ	073h
kVars	equ	074h
kCatalog	equ	075h
kQuit	equ	076h
kLastEnt	equ	077h
kLinkIO	equ	078h
kMem	equ	079h
kList	equ	07Ah
kVector	equ	07Bh
kConst	equ	07Ch
kMatrix	equ	07Dh
kPoly	equ	07Eh
kSimult	equ	07Fh
kStat	equ	080h
kGrMenu	equ	081h
kMode	equ	082h
kPrgm	equ	083h
kCalcu	equ	084h
kSolver	equ	085h
;
kTable	equ	086h		; new key added for LEO
;
kBOL	equ	087h
kEOL	equ	088h
kCUSTM	equ	093h
kF1	equ	0C2h
kF2	equ	0C3h
kF3	equ	0C4h
kF4	equ	0C5h
kF5	equ	0C6h
kF6	equ	0C7h
kF7	equ	0C8h
kF8	equ	0C9h
kF9	equ	0CAh
kF10    equ     0CBh
kGRAPH	equ	0FFh
