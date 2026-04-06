.ifdef PROLOGUE_CPUID_PPRO
BOM     MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
.endif

.ifdef PROLOGUE_CPUID
BOM                    UOP.000         (ALIAS.014     , ALIAS.014      )
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
.endif

.ifdef PROLOGUE_UJCC
        U_JCC.NT.Z      (ALIAS.1ad     , next_uop     , IA.11 , U2.08 , U3.1b )
next_uop:
EOM.Fl2 UOP.0D8         (CONST_0       , EIP_30         , U2.20 )
        MOVE.DSZ32(CONST         , CONST_0)
        MOVE.DSZ32( CONST , CONST_0 )
        MOVE.DSZ32( CONST , CONST_0 )
.endif

.ifdef PROLOGUE_SIGEVENT
        SIGEVENT       (CONST_16+0AB  , CONST_16+0AB  , U2.08)
        UOP.0D8         (CONST_0       , EIP_30        , U2.20)
EOM.Fl2 UOP.0D4         (CONST_0       , CONST_0       )
        MOVE.DSZ32( CONST , CONST_0 )
        MOVE.DSZ32( CONST , CONST_0 )
.endif

.ifdef PROLOGUE_CPUID_FULL
; this code runs on platforms with cpuid < 0x652, likely the PLA puts entry point to CPUID instruction into MSRAM
BOM     UOP.000         (ALIAS.014     , ALIAS.014      )
        STRD.DSZ32      (CONST_0       , CONST_0        )
        UOP.134         (CONST_6  , CONST_6   )
        U_JCC.NT.Z      (ALIAS.021     , CPUID_SKIP , IA.11 , U3.1b ) ; Never taken? Serialize?
        MOVE.DSZ32( CONST , CONST_0 ) ; fall through to our code
.endif
