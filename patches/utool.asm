.org 0x3FAC

.include "patches/prologue.inc"

new_patch_start:
        TMPC = SUB.DSZ32 ( EAX , CONST_0 )
        U_JCC.NT.NZ     ( TMPC , cpuid_try_leaf42, IA.11, U3.1b )

        EAX = MOVE.DSZ32( CONST, CONST_16+002 )

        EBX = MOVE.DSZ32( CONST, CONST_16+065 )
        EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
        EBX = OR.DSZ32  ( EBX,   CONST_16+074 )
        EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
        EBX = OR.DSZ32  ( EBX,   CONST_16+065 )
        EBX = SHL.DSZ32 ( EBX,   CONST_16+008 )
        EBX = OR.DSZ32  ( EBX,   CONST_16+070 )

        EDX = MOVE.DSZ32( CONST, CONST_16+06f )
        EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
        EDX = OR.DSZ32  ( EDX,   CONST_16+06a )
        EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
        EDX = OR.DSZ32  ( EDX,   CONST_16+062 )
        EDX = SHL.DSZ32 ( EDX,   CONST_16+008 )
        EDX = OR.DSZ32  ( EDX,   CONST_16+072 )

        ECX = MOVE.DSZ32( CONST, CONST_16+020 ) 
        ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
        ECX = OR.DSZ32  ( ECX,   CONST_16+078 )
        ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
        ECX = OR.DSZ32  ( ECX,   CONST_16+06e )
        ECX = SHL.DSZ32 ( ECX,   CONST_16+008 )
        ECX = OR.DSZ32  ( ECX,   CONST_16+072 )
        U_JMP.NT        ( CONST, CPUID_DONE, IA.11, U3.1b )

cpuid_try_leaf42:
        TMPC = SUB.DSZ32 ( EAX,   CONST_16+042)
        U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf43, IA.11, U3.1b )
        MOVE.DSZ32      ( CONST         , CONST_0 )

cpuid_leaf42:
        MOVETOCREG      (CONST_0e+188, EBX, U2.08)          
        EAX = MOVEFROMCREG( CONST_0e+189, CONST, U2.20 )     
        U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )  

cpuid_try_leaf43:
        TMPC = SUB.DSZ32 ( EAX,   CONST_16+043)	           
        U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf44, IA.11, U3.1b )
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_leaf43:
        MOVETOCREG      ( ECX, EBX )
        U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )  
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_try_leaf44:
        TMPC = SUB.DSZ32 ( EAX,   CONST_16+044 )
        U_JCC.NT.NZ     ( TMPC,   cpuid_try_leaf45, IA.11, U3.1b )
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_leaf44:
        EAX = MOVEFROMCREG( ECX, CONST_06+000 )
        U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_try_leaf45:
        TMPC = SUB.DSZ32 ( EAX,   CONST_16+045 )
        U_JCC.NT.NZ     ( TMPC,   cpuid_try_other, IA.11, U3.1b )
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_leaf45:
        TMP0 = FREADROM ( CONST, ECX )
        EAX  = MOVE.DSZ32 ( CONST, TMP0 )
        EDX  = INTEXTRACT.HI32( TMP0, CONST_0 )
        U_JMP.NT        ( CONST, cpuid_leaf4x_done, IA.11, U3.1b )
        MOVE.DSZ32     (CONST         , CONST_0)
        MOVE.DSZ32     (CONST         , CONST_0)

cpuid_try_other:
        EBX = MOVE.DSZ32 (CONST, CONST_0)
        ECX = MOVE.DSZ32 (CONST, CONST_0)
        U_JMP.NT        ( CONST, CPUID_OTHER, IA.11, U3.1b )
        MOVE.DSZ32 (CONST, CONST_0)

cpuid_leaf4x_done:
        ECX = MOVE.DSZ32( CONST , CONST_16+042 )
        U_JMP.NT        ( CONST, CPUID_DONE, IA.11, U3.1b )
