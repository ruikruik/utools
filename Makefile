.PHONY: UCODE_PII UCODE_PIII clean all

#TODO: Enable -Wconversion one day...
CFLAGS += $(CFLAGS-$@) -O2 -Wall -Wextra

CFLAGS-utool = -m32 -static
CFLAGS-microload = -m32  -static

PII_UCPUID = 634 652 66a
PII_DCPUID = 66a 634

PII_DATS += $(foreach cpuid,$(PII_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PII_DATS += $(foreach cpuid,$(PII_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

PIII_DATS += $(foreach cpuid,$(PIII_UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
PIII_DATS += $(foreach cpuid,$(PIII_DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

DATS = $(PII_DATS) $(PIII_DATS)

GENFILES = $(DATS)
GENFILES += $(DATS:.dat=.gen)
GENFILES += $(DATS:.dat=.hex)

EXEC := microload utool msrom2scramble
DOTEXE := $(EXEC:%=%.exe)

$(PII_DATS) : EXTRA_OPTS = -t pentium2
$(PIII_DATS) : EXTRA_OPTS = -t pentium3

all:  $(EXEC) $(PII_DATS) $(PIII_DATS)

microload : microload.c

%.dat: %.txt %.hex
	../patchtools_pub/patchtools -c -p $@ -i $(filter %.txt,$<)

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

%.uhex : %.gen
	python ../p6tools/simpleas.py $< > $@
#	./p6as $(EXTRA_OPTS) $< > $@

%.hex : %.uhex
	python ../p6tools/scramble.py $< > $@
#	./p6scrambler $(EXTRA_OPTS) $< > $@

utool : utool.c

msrom2scramble : msrom2scramble.c

clean :
	rm -f $(UCODE) $(GENFILES) $(UCODE) $(DOTEXE) $(EXEC)
