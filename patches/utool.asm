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

.ifdef PROLOGUE_CPUID_FULL
; this code runs on platforms with cpuid < 0x652, likely the PLA puts entry point to CPUID instruction into MSRAM
UROM_3FAC	 BOM                     UOP.000         (ALIAS.014     , ALIAS.014      )
UROM_3FAD                                STRD.DSZ32      (CONST_0       , CONST_0        )
UROM_3FAE                                UOP.134         (CONST_6  , CONST_6   )
UROM_3FB0                                U_JCC.NT.Z      (ALIAS.021     , CPUID_SKIP , IA.11 , U3.1b ) ; Never taken? Serialize?
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

UROM_3FB2 new_patch_start:
UROM_3FB2	                          TMPC = SUB.DSZ32 ( EAX , CONST_0 )
UROM_3FB4	                          U_JCC.NT.NZ     ( TMPC , cpuid_try_leaf42, IA.11, U3.1b )

UROM_3FB5	                          EAX = MOVE.DSZ32( CONST, CONST_16+002 )

UROM_3FB6	                          EBX = MOVE.DSZ32( CONST, CONST_16+065 )
UROM_3FB8	                          EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
UROM_3FB9	                          EBX = OR.DSZ32  ( EBX,   CONST_16+074 )
UROM_3FBA	                          EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
UROM_3FBC	                          EBX = OR.DSZ32  ( EBX,   CONST_16+065 )
UROM_3FBD	                          EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
UROM_3FBE	                          EBX = OR.DSZ32  ( EBX,   CONST_16+070 )

UROM_3FC0	                          EDX = MOVE.DSZ32( CONST, CONST_16+06f )
UROM_3FC1	                          EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
UROM_3FC2                                 EDX = OR.DSZ32  ( EDX,   CONST_16+06a )
UROM_3FC4	                          EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
UROM_3FC5	                          EDX = OR.DSZ32  ( EDX,   CONST_16+062 )
UROM_3FC6	                          EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
UROM_3FC8	                          EDX = OR.DSZ32  ( EDX,   CONST_16+072 )

UROM_3FC9	                          ECX = MOVE.DSZ32( CONST, CONST_16+020 ) 
UROM_3FCA	                          ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
UROM_3FCC	                          ECX = OR.DSZ32  ( ECX,   CONST_16+078 )
UROM_3FCD	                          ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
UROM_3FCE	                          ECX = OR.DSZ32  ( ECX,   CONST_16+06e )
UROM_3FD0	                          ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
UROM_3FD1	                          ECX = OR.DSZ32  ( ECX,   CONST_16+072 )

UROM_3FD2	                          U_JMP.NT        ( CONST, CPUID_DONE, IA.11, U3.1b )

UROM_3FD4  cpuid_try_leaf42:
UROM_3FD4	                          TMPC = SUB.DSZ32 ( EAX,   CONST_16+042)
UROM_3FD5	                          U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf43, IA.11, U3.1b )
UROM_3FD6	                          MOVE.DSZ32      ( CONST         , CONST_0 )

UROM_3FD8  cpuid_leaf42:	 
UROM_3FD8	                          MOVETOCREG      (CONST_0e+188, EBX, U2.08)          
UROM_3FD9	                          EAX = MOVEFROMCREG( CONST_0e+189, CONST, U2.20 )     
UROM_3FDA	                          U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )  

UROM_3FDC  cpuid_try_leaf43:  
UROM_3FDC	                          TMPC = SUB.DSZ32 ( EAX,   CONST_16+043)	           
UROM_3FDD	                          U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf44, IA.11, U3.1b )
UROM_3FDE	                          MOVE.DSZ32     (CONST         , CONST_0)

UROM_3FE0  cpuid_leaf43:
UROM_3FE0	                          MOVETOCREG      ( ECX, EBX )
UROM_3FE1	                          U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )  
UROM_3FE2	                          MOVE.DSZ32     (CONST         , CONST_0)

UROM_3FE4  cpuid_try_leaf44:
UROM_3FE4	                          TMPC = SUB.DSZ32 ( EAX,   CONST_16+044 )
UROM_3FE5	                          U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf45, IA.11, U3.1b )
UROM_3FE6	                          MOVE.DSZ32     (CONST         , CONST_0)

UROM_3FE8  cpuid_leaf44:
UROM_3FE8	                          EAX = MOVEFROMCREG( ECX, CONST_06+000 )
UROM_3FE9	                          U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )
UROM_3FEA	                          MOVE.DSZ32     (CONST         , CONST_0)

UROM_3FEC  cpuid_try_leaf45:
UROM_3FEC	                          TMPC = SUB.DSZ32 ( EAX,   CONST_16+045 )
UROM_3FED	                          U_JCC.NT.NZ     ( TMPC,   cpuid_try_other, IA.11, U3.1b )
UROM_3FEE	                          MOVE.DSZ32     (CONST         , CONST_0)

UROM_3FF0  cpuid_leaf45:
UROM_3FF0	                          TMP0 = FREADROM ( CONST, ECX )
UROM_3FF1	                          EAX  = MOVE.DSZ32 ( CONST, TMP0 )
UROM_3FF2	                          EDX  = INTEXTRACT.HI32( TMP0, CONST_0 )
UROM_3FF4	                          U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )
UROM_3FF5	                          MOVE.DSZ32     (CONST         , CONST_0)
UROM_3FF6	                          MOVE.DSZ32     (CONST         , CONST_0)
UROM_3FF8  cpuid_try_other:
UROM_3FF8	                          EBX = MOVE.DSZ32 (CONST, CONST_0)
UROM_3FF9	                          ECX = MOVE.DSZ32 (CONST, CONST_0)
UROM_3FFA	                          U_JMP.NT        ( CONST, CPUID_OTHER, IA.11, U3.1b )
UROM_3FFC	                          MOVE.DSZ32 (CONST, CONST_0)
UROM_3FFD  cpuid_leaf4x_done:
UROM_3FFD	                          ECX = MOVE.DSZ32( CONST , CONST_16+042 )
UROM_3FFE	                          U_JMP.NT        ( CONST, CPUID_DONE, IA.11, U3.1b )
