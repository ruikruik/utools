.org 0x3FAC

.include "patches/prologue.inc"

;Temporary hack to map MSR 0x8b to right place on PIII
.ifdef PENTIUMIII
.define MSR8B_L CONST_0E+0C6
.define MSR8B_H CONST_0E+0C7
.else
.define MSR8B_L BBL_CR_D3_L
.define MSR8B_H BBL_CR_D3_H
.endif

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

        TMP0 = MOVEFROMCREG   (MSR8B_H   , MSR8B_H)
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

        TMPD = MOVE.DSZ32 (CONST, CONST_16+001)
        TMPD = SHL.DSZ32      (TMPD , CONST_16+009   ) ; TMPD = 0x200
        TMPD = SUB.DSZ32      (TMPC , TMPD )

        U_JCC.NT.NZ    ( TMPD , loop, IA.11, U3.1b )
        ; On 0x6b4 the write to lower portion to MSR8B_L needs to either be
        ; interleaved by a U_JMP.NT, or at least 2 triads far away from write MSR8B_L,
        ; otherwise the first write to MSR8B_H gets discarded somehow.
        ; The magic value U2.28 seems to fix it as well! Note that both bits are needed.
        TMPC = MOVE.DSZ32     (CONST , CONST_0 )
        MOVETOCREG     (MSR8B_H , TMPC, U2.28 )
        TMPC = MOVE.DSZ32     (CONST , CONST_16+bc )
        MOVETOCREG     (MSR8B_L , TMPC, U2.28 )
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
skip:
        TMPC = MOVE.DSZ32     (CONST , CONST_16+042 )
        MOVETOCREG     (MSR8B_L , TMPC )
        U_JMP.NT        ( CONST, ROMDUMPER_DONE, IA.11, U3.1b )
