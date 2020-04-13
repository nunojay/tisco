;
;
;       Floating Point Math entry points for TI-86
;
;
_INTGR          equ            5468h            ; op1 = intgr(op1)
_MINUS1         equ            5470h            ; op1 = op1 - 1
_FPSUB          equ            5474h            ; op1 = op1-op2
_FPADD          equ            5478h            ; op1 = op1+op2
_TIMESPT5       equ            5484h            ; op1 = .5 * op1
_FPSQUARE       equ            5488h            ; op1 = op1^2
_FPMULT         equ            548Ch            ; op1 = op1*op2
_FRAC           equ            549Ch            ; op1 = frac(op1)
_FPRECIP        equ            54A4h            ; op1 = 1/op1
_FPDIV          equ            54A8h            ; op1 = op1/op2
;
_SQROOT         equ            54ACh     ; op1 = sqrt(op1), neg #'s error
_SQROOTP        equ            54B0h     ; op1 = sqrt(op1), neg #'s -> complex
;
_RNDGUARD       equ            54BCh            ; round op1 to 12 digits
;
_LNX            equ            54C4h     ; op1 = ln(op1), neg #'s error
_LNXP           equ            54C8h     ; op1 = ln(op1), neg #'s -> complex
;
_LOGXP          equ            54CCh     ; op1 = log(op1), neg #'s error
_LOGX           equ            54D0h     ; op1 = log(op1), neg #'s -> complex
;
_ETOX           equ            54D4h            ; op1 = e^op1
_TENX           equ            54D8h            ; op1 = 10^op1
;
_SIN            equ            54E0h            ; op1 = sin(op1), current mode
_COS            equ            54E4h            ; op1 = cos(op1), current mode
_TAN            equ            54E8h            ; op1 = tan(op1), current mode
;
_TANH           equ            54F0h            ; op1= hyp tan(op1)
_COSH           equ            54F4h            ; op1= hyp cos(op1)
_SINH           equ            54F8h            ; op1= hyp sin(op1)
;
_ACOS           equ            5508h   ; op1 = inv cos(op1), no complex results
_ACOSP          equ            550Ch   ; op1 = inv cos(op1), complex results ok
;
_ATAN           equ            5510h            ; op1 = inv tan(op1)
_ASIN           equ            5514h            ; op1 = inv sin(op1)
;
_ATANH          equ            551Ch            ; op1 = inv hyp Tan(op1)
_ASINH          equ            5524h            ; op1 = inv hyp sin(op1)
_ACOSH          equ            5528h            ; op1 = inv hyp cos(op1)
;
_YTOX           equ            5538h            ; op1 = op1^op2

