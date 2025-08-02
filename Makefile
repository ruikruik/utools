
CFLAGS += $(CFLAGS-$@) -O2 -Wall -Wextra

CFLAGS-utool = -m32
CFLAGS-microload = -m32

UCPUID := 634 652 66a
DCPUID := 66a 634

UCODE := $(UCPUID:%=patches/%/utool-ucode.gen)
UCODE += $(UCPUID:%=patches/%/utool-ucode.hex)

UCODE += $(DCPUID:%=patches/%/msromdumper-ucode.gen)
UCODE += $(DCPUID:%=patches/%/msromdumper-ucode.hex)

UCODE += $(foreach cpuid,$(UCPUID),patches/$(cpuid)/utool-$(cpuid).dat)
UCODE += $(foreach cpuid,$(DCPUID),patches/$(cpuid)/msromdumper-$(cpuid).dat)

EXEC := microload utool msrom2scramble
DOTEXE := $(EXEC:%=%.exe)

all: $(EXEC) $(UCODE)

microload : microload.c

patches/634/msromdumper-ucode.gen: patches/ucode-entry-bom.txt patches/msromdumper-ucode.txt
	cat patches/ucode-entry-bom.txt > $(@D)/msromdumper-ucode.gen
	sed -e 's/ROMDUMPER_DONE/UROM_09fa/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-ucode.gen

patches/66a/msromdumper-ucode.gen: patches/ucode-exit.txt patches/msromdumper-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/msromdumper-ucode.gen
	sed -e 's/ROMDUMPER_DONE/UROM_2B7A/g' < patches/msromdumper-ucode.txt >> $(@D)/msromdumper-ucode.gen

patches/66a/utool-ucode.gen: patches/ucode-entry.txt patches/utool-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1CD1/g' -e 's/UROM_CPUID_OTHER/UROM_2B8A/g' < patches/utool-ucode.txt >> $(@D)/utool-ucode.gen

patches/652/utool-ucode.gen: patches/ucode-exit.txt patches/utool-ucode.txt
	cat patches/ucode-exit.txt > $(@D)/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1E15/g' -e 's/UROM_CPUID_OTHER/UROM_2F10/g' < patches/utool-ucode.txt >> $(@D)/utool-ucode.gen

patches/634/utool-ucode.gen: patches/ucode-entry.txt patches/utool-ucode.txt
	sed -e 's/UROM_CPUID_SKIP/UROM_1e21/g' <  patches/ucode-entry.txt > $(@D)/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1401/g' -e 's/UROM_CPUID_OTHER/UROM_0A0A/g' < patches/utool-ucode.txt >> $(@D)/utool-ucode.gen

%.dat : %.txt
	../patchtools_pub/patchtools -c -p $@ -i $<

%.uhex : %.gen
	python ../p6tools/simpleas.py $< > $@

%.hex : %.uhex
	python ../p6tools/scramble.py $< > $@

utool : utool.c

msrom2scramble : msrom2scramble.c

clean :
	rm -f $(UCODE) $(DOTEXE) $(EXEC)
