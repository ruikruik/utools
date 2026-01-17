.PHONY: clean all

#TODO: Enable -Wconversion one day...
CFLAGS += $(CFLAGS-$@) -O2 -Wall -Wextra

CFLAGS-utool = -m32 -static
CFLAGS-microload = -m32  -static

PPRO_DCPUID = 612 619

PII_UCPUID = 634 652 66a
PII_DCPUID = 66a 634 651

PIII_UCPUID = 686

PM_UCPUID = 6d8

PPRO_DATS += $(foreach cpuid,$(PPRO_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PPRO_DATS += $(foreach cpuid,$(PPRO_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

PII_DATS += $(foreach cpuid,$(PII_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PII_DATS += $(foreach cpuid,$(PII_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

PIII_DATS += $(foreach cpuid,$(PIII_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PIII_DATS += $(foreach cpuid,$(PIII_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

PM_DATS += $(foreach cpuid,$(PM_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PM_DATS += $(foreach cpuid,$(PM_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

DATS = $(PPRO_DATS) $(PII_DATS) $(PIII_DATS) $(PM_DATS)

GENFILES = $(DATS)
GENFILES += $(DATS:.dat=.gen)
GENFILES += $(DATS:.dat=.hex)

UHEX = $(DATS:.dat=.uhex)

EXEC := microload utool msrom2scramble
DOTEXE := $(EXEC:%=%.exe)

$(PPRO_DATS) : EXTRA_OPTS = -t pentiumpro
$(PII_DATS) : EXTRA_OPTS = -t pentium2
$(PIII_DATS) : EXTRA_OPTS = -t pentium3
$(PM_DATS) : EXTRA_OPTS = -t pentiumm

.precious: $(GENFILES)  $(UHEX)

all:  $(EXEC) $(DATS)

microload : microload.c

%.dat: %.txt %.hex
	../patchtools_pub/patchtools -c -p $@ -i $(filter %.txt,$<)

patches/651/msromdumper-651.gen: patches/ucode-entry-bom.txt patches/msromdumper-ucode.txt
	cat patches/ucode-entry-bom.txt > $(@D)/msromdumper-651.gen
	sed -e 's/ROMDUMPER_DONE/UROM_0f5c/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-651.gen

patches/634/msromdumper-634.gen: patches/ucode-entry-bom.txt patches/msromdumper-ucode.txt
	cat patches/ucode-entry-bom.txt > $(@D)/msromdumper-634.gen
	sed -e 's/ROMDUMPER_DONE/UROM_09fa/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-634.gen

patches/66a/msromdumper-66a.gen: patches/ucode-exit.txt patches/msromdumper-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/msromdumper-66a.gen
	sed -e 's/ROMDUMPER_DONE/UROM_2B7A/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-66a.gen

patches/66a/utool-66a.gen: patches/ucode-entry.txt patches/utool-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/utool-66a.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1CD1/g' -e 's/UROM_CPUID_OTHER/UROM_2B8A/g' < patches/utool-ucode.txt >> $(@D)/utool-66a.gen

patches/652/utool-652.gen: patches/ucode-exit.txt patches/utool-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/utool-652.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1E15/g' -e 's/UROM_CPUID_OTHER/UROM_2F10/g' < patches/utool-ucode.txt >> $(@D)/utool-652.gen

patches/634/utool-634.gen: patches/ucode-entry.txt patches/utool-ucode.txt
	sed -e 's/UROM_CPUID_SKIP/UROM_1e21/g' <  patches/ucode-entry.txt > $(@D)/utool-634.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1401/g' -e 's/UROM_CPUID_OTHER/UROM_0A0A/g' < patches/utool-ucode.txt >> $(@D)/utool-634.gen

patches/6d8/utool-6d8.gen: patches/ucode-exit-pm.txt patches/utool-ucode.txt
	cat patches/ucode-exit-pm.txt > $(@D)/utool-6d8.gen
	sed -e 's/UROM_CPUID_DONE/UROM_332C/g' -e 's/UROM_CPUID_OTHER/UROM_1B3E/g' < patches/utool-ucode.txt >> $(@D)/utool-6d8.gen

#The prologue patches/ucode-exit-pm.txt is unused/can be replaced with nops 
patches/686/utool-686.gen: patches/ucode-exit-pm.txt patches/utool-ucode.txt
	cat patches/ucode-exit-pm.txt > $(@D)/utool-686.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1B9C/g' -e 's/UROM_CPUID_OTHER/UROM_07BE/g' < patches/utool-ucode.txt >> $(@D)/utool-686.gen

patches/619/msromdumper-619.gen: patches/ucode-entry-bom-ppro.txt patches/msromdumper-ucode.txt
	cat patches/ucode-entry-bom-ppro.txt > $(@D)/msromdumper-619.gen
	sed -e 's/ROMDUMPER_DONE/UROM_291D/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-619.gen

patches/612/msromdumper-612.gen: patches/ucode-entry-bom-ppro.txt patches/msromdumper-ucode.txt
	cat patches/ucode-entry-bom-ppro.txt > $(@D)/msromdumper-612.gen
	sed -e 's/ROMDUMPER_DONE/UROM_291D/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-612.gen

%.uhex : %.gen
#	python ../p6tools/simpleas.py $< > $@
	../p6microcode-tools/p6as $(EXTRA_OPTS) $< > $@

%.hex : %.uhex
#	python ../p6tools/scramble.py $< > $@
	../p6microcode-tools/p6scrambler $(EXTRA_OPTS) $< > $@

utool : utool.c

msrom2scramble : msrom2scramble.c

clean :
	rm -f $(UCODE) $(GENFILES) $(UCODE) $(DOTEXE) $(EXEC)
