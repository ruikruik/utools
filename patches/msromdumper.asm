.org 0x3FAC

.include "patches/prologue.inc"

; Dump the MSROM into RAM using high 32 bits of MSR 0x8b as a linear address where to place it
; low 32-bits of MSR 0x8b serves as debug.

        TMP0 = MOVEFROMCREG   (BBL_CR_D3_H   , BBL_CR_D3_H)
        TMP0 = SUB.DSZ32 ( TMP0 , CONST_16 )
        U_JCC.NT.Z     (TMP0 , skip, IA.11, U3.1b )
        TMPC        =  MOVE.DSZ32     (CONST_0        , CONST_0        )
        TMPC = MOVETOCREG     (MS_CR_ADDR    , TMPC) ; IMPORTANT: the TMPC is kind of dependency so that previous op won't execute until it finished
        TMPC = MOVEFROMCREG   (MS_CR_DATA    , TMPC) ; IMPORTANT: the TMPC is kind of dependency so that previous op won't execute until it finished
        TMPC = MOVE.DSZ32 (CONST, CONST_16+001)
        TMPC =  SHL.DSZ32      (TMPC           , CONST_16+00f   )
loop:
        TMP1 = MOVEFROMCREG   (MS_CR_DATA    , TMPC ) ; IMPORTANT: the TMPC is kind of dependency so that previous op won't execute until it finished
.ifdef PENTIUMM
        STRD.DSZ32           (TMP1, CONST_0)
.else
        STRD.DSZ32           (CONST_0,     TMP1)
.endif
        STA.M40.SC1.DSZ32(CONST_06  , TMP0           , LINSEG   )
        TMP0 = ADD.DSZ32       (TMP0          , CONST_16+004   )
        TMPC        =  SUB.DSZ32       (TMPC          , CONST_16+001   )
        U_JCC.NT.NZ     ( TMPC , loop, IA.11, U3.1b )
        TMPC        =  MOVE.DSZ32     (CONST         , CONST_0        )
        MOVETOCREG     (BBL_CR_D3_H    , TMPC)
        TMPC        =  MOVE.DSZ32     (CONST        , CONST_16+bc)
        MOVETOCREG     (BBL_CR_D3_L    , TMPC)
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
skip:
        TMPC        =  MOVE.DSZ32     (CONST        , CONST_16+042)
        MOVETOCREG     (BBL_CR_D3_L    , TMPC)
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )

        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.08 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.10 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.10 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.18 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.18 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.18 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.10 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.08 )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        )
        MOVE.DSZ32     (CONST         , CONST_0        , U3.10 )
