.org 0x3FAC

.include "patches/prologue.inc"

; Dump the full FPROM (FREADROM) into RAM using the high 32 bits of MSR 0x8b as
; the linear buffer address, as msromdumper does. Low 32 bits of MSR 0x8b
; report status: 0xbc = done, 0x42 = no buffer given.
;
; For each index 0..0x1ff, one 64-byte record of DWORDs:
;   +00 low32   MOVE.DSZ32 of the FREADROM value
;   +04 hi32    INTEXTRACT.HI32
;   +08 up32    INTEXTRACT.UP32
;   +0c..+28    UOP.060 .. UOP.067
;   +2c..+3c    untouched (left as the caller's fill pattern)
; Raw register values are exported; no field meanings are assumed.
; Tested on a 0619 Pentium Pro only.

        TMP0 = MOVEFROMCREG   (BBL_CR_D3_H   , BBL_CR_D3_H)
        TMPD = SUB.DSZ32      (TMP0 , CONST_16 )
        U_JCC.NT.Z     (TMPD , skip, IA.11, U3.1b )
        TMPC = MOVE.DSZ32     (CONST , CONST_0 )               ; FPROM index
loop:
        TMP2 = FREADROM       (CONST , TMPC )
        TMP1 = MOVE.DSZ32     (CONST , TMP2 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+000 , TMP0 , LINSEG )
        TMP1 = INTEXTRACT.HI32 (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+004 , TMP0 , LINSEG )
        TMP1 = INTEXTRACT.UP32 (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+008 , TMP0 , LINSEG )
        TMP1 = UOP.060        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+00c , TMP0 , LINSEG )
        TMP1 = UOP.061        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+010 , TMP0 , LINSEG )
        TMP1 = UOP.062        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+014 , TMP0 , LINSEG )
        TMP1 = UOP.063        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+018 , TMP0 , LINSEG )
        TMP1 = UOP.064        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+01c , TMP0 , LINSEG )
        TMP1 = UOP.065        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+020 , TMP0 , LINSEG )
        TMP1 = UOP.066        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+024 , TMP0 , LINSEG )
        TMP1 = UOP.067        (TMP2 , CONST_0 )
        STRD.DSZ32            (CONST_0 , TMP1 )
        STA.M40.SC1.DSZ32     (CONST_04+028 , TMP0 , LINSEG )
        TMP0 = ADD.DSZ32      (TMP0 , CONST_16+040 )           ; next 64-byte record
        TMPC = ADD.DSZ32      (TMPC , CONST_16+001 )
        TMPD = SUB.DSZ32      (TMPC , CONSTROM.015 )           ; CONSTROM.015 = 0x200 on 0619
        U_JCC.NT.NZ    ( TMPD , loop, IA.11, U3.1b )
        TMPC = MOVE.DSZ32     (CONST , CONST_0 )
        MOVETOCREG     (BBL_CR_D3_H , TMPC )
        TMPC = MOVE.DSZ32     (CONST , CONST_16+bc )
        MOVETOCREG     (BBL_CR_D3_L , TMPC )
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
skip:
        TMPC = MOVE.DSZ32     (CONST , CONST_16+042 )
        MOVETOCREG     (BBL_CR_D3_L , TMPC )
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
