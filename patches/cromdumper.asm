.ifdef PROLOGUE_CPUID_PPRO
UROM_3FAC	 BOM                     MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FAD                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FAE                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FB0                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FB1                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
.endif

.ifdef PROLOGUE_CPUID
UROM_3FAC        BOM                     UOP.000         (ALIAS.014     , ALIAS.014      )
UROM_3FAD                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FAE                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FB0                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
UROM_3FB1                                MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
.endif

.ifdef PROLOGUE_UJCC
UROM_3FAC                                 U_JCC.NT.Z      (ALIAS.1ad     , UROM_3FAD      , IA.11 , U2.08 , U3.1b )
UROM_3FAD addr_3FAD:
UROM_3FAD       EOM.Fl2                   UOP.0D8         (CONST_0       , EIP_30         , U2.20 )
UROM_3FAE                                MOVE.DSZ32(CONST         , CONST_0)
UROM_3FB0                                MOVE.DSZ32( CONST , CONST_0 )
UROM_3FB1                                MOVE.DSZ32( CONST , CONST_0 )
.endif

.ifdef PROLOGUE_SIGEVENT
UROM_3FAC	                          SIGEVENT       (CONST_16+0AB  , CONST_16+0AB  , U2.08)
UROM_3FAD	                          UOP.0D8         (CONST_0       , EIP_30        , U2.20)
UROM_3FAE	EOM.Fl2                   UOP.0D4         (CONST_0       , CONST_0       )
UROM_3FB0                                MOVE.DSZ32( CONST , CONST_0 )
UROM_3FB1                                MOVE.DSZ32( CONST , CONST_0 )
.endif

; Dump the MSROM into RAM using high 32 bits of MSR 0x8b as a linear address where to place it
; low 32-bits of MSR 0x8b serves as debug.
.org 0x3fb2
    TMP0 = MOVEFROMCREG   (BBL_CR_D3_H   , BBL_CR_D3_H)
    TMP0 = SUB.DSZ32 ( TMP0 , CONST_16 )
    U_JCC.NT.NZ     (TMP0 , dodump, IA.11, U3.1b )
    TMPC        =  MOVE.DSZ32     (CONST        , CONST_16+042)
    MOVETOCREG     (BBL_CR_D3_L    , TMPC)
    U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )

 dodump:
UROM_3FBA  TMPC        =  MOVE.DSZ32     (CONST         , CONST_0        )

.org 0x3fbc

; put here
;    STRD.DSZ32           (CONST_0       , CONSTROM.000 )
;    STA.M40.SC1.DSZ64(CONST_04+0  , TMP0          , LINSEG  )
;    ...
;    end with:

;    MOVETOCREG     (BBL_CR_D3_H    , TMPC)
;    U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
