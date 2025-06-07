/*
 * This program transforms the microcode ROM format read out from CRBUS MS_CR_ADDR / MS_CR_DATA
 * register pair to the "scrambled" format of microcode used in the microcode updates.
 *
 * It is unknown why there is yet another scrambled form. Obfuscation? Selftest? Sillicon?
 *
 * Both tables below represent the bits as seen in ROM format.
 * Comments represent what was put as input in microcode update "scrambled" format
 * 
 * The inverse transform computed and stored in dw2bit and dw2dw arrays
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


uint32_t dw_masks[][8] = {
   /* 0xffffffff  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000 */
    { 0x00007000, 0x00007000, 0x00003000, 0x00003000, 0x00003000, 0x00003800, 0x00003800, 0x00003800 },
   /* 0x00000000  0xffffffff  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000  0x00000000 */
    { 0x00000F00, 0x00000F00, 0x00000F00, 0x00000F00, 0x00000F00, 0x00000700, 0x00000780, 0x00000780 },
    { 0x000000F0, 0x000000F0, 0x000000F0, 0x000000F0, 0x000000F0, 0x000000F0, 0x00000070, 0x00000078 },
    { 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x00000007 },
    { 0x00070000, 0x00070000, 0x00070000, 0x00038000, 0x00038000, 0x00038000, 0x00038000, 0x00038000 },
    { 0x00780000, 0x00780000, 0x00380000, 0x003C0000, 0x003C0000, 0x003C0000, 0x003C0000, 0x003C0000 },
    { 0x07800000, 0x03800000, 0x03C00000, 0x03C00000, 0x03C00000, 0x03C00000, 0x03C00000, 0x03C00000 },
    { 0x38000000, 0x3C000000, 0x3C000000, 0x3C000000, 0x3C000000, 0x3C000000, 0x3C000000, 0x3C000000 }
};

uint32_t dw_to_crbusrom[][8] = {
   /* 0x00000001  0x00000001  0x00000001  0x00000001  0x00000001  0x00000001  0x00000001  0x00000001 */
    { 0x00000000, 0x04000000, 0x00400000, 0x00040000, 0x00000800, 0x00000080, 0x00000008, 0x00000000 },
   /* 0x00000002  0x00000002  0x00000002  0x00000002  0x00000002  0x00000002  0x00000002  0x00000002 */
    { 0x00000000, 0x00000000, 0x04000000, 0x00400800, 0x00040080, 0x00000008, 0x00000000, 0x00000000 },
    { 0x00000000, 0x00000000, 0x00000800, 0x04000080, 0x00400008, 0x00040000, 0x00000000, 0x00000000 },
    { 0x00000000, 0x00000800, 0x00000080, 0x00000008, 0x04000000, 0x00400000, 0x00040000, 0x00000000 },
    { 0x00000800, 0x00000080, 0x00000008, 0x00000000, 0x00000000, 0x04000000, 0x00400000, 0x00040000 },
    { 0x00080080, 0x00000008, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000000, 0x00400400 },
    { 0x00800008, 0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000400, 0x04000040 },
    { 0x08000000, 0x00800000, 0x00080000, 0x00008000, 0x00000000, 0x00000400, 0x00000040, 0x00000004 },
    { 0x00000000, 0x08000000, 0x00800000, 0x00080000, 0x00008400, 0x00000040, 0x00000004, 0x00000000 },
    { 0x00000000, 0x00000000, 0x08000000, 0x00800400, 0x00080040, 0x00008004, 0x00000000, 0x00000000 },
    { 0x00000000, 0x00004000, 0x00000400, 0x08000040, 0x00800004, 0x00080000, 0x00008000, 0x00000000 },
    { 0x00004000, 0x00000400, 0x00000040, 0x00000004, 0x08000000, 0x00800000, 0x00080000, 0x00008000 },
    { 0x00010400, 0x00000040, 0x00000004, 0x00000000, 0x00000000, 0x08000000, 0x00800000, 0x00082000 },
    { 0x00100040, 0x00010004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08002000, 0x00800200 },
    { 0x01000004, 0x00100000, 0x00010000, 0x00000000, 0x00000000, 0x00002000, 0x00000200, 0x08000020 },
    { 0x10000000, 0x01000000, 0x00100000, 0x00010000, 0x00002000, 0x00000200, 0x00000020, 0x00000002 },
    { 0x00000000, 0x10000000, 0x01000000, 0x00102000, 0x00010200, 0x00000020, 0x00000002, 0x00000000 },
    { 0x00000000, 0x00000000, 0x10002000, 0x01000200, 0x00100020, 0x00010002, 0x00000000, 0x00000000 },
    { 0x00000000, 0x00002000, 0x00000200, 0x10000020, 0x01000002, 0x00100000, 0x00010000, 0x00000000 },
    { 0x00002000, 0x00000200, 0x00000020, 0x00000002, 0x10000000, 0x01000000, 0x00100000, 0x00010000 },
    { 0x00020200, 0x00000020, 0x00000002, 0x00000000, 0x00000000, 0x10000000, 0x01000000, 0x00101000 },
    { 0x00200020, 0x00020002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10001000, 0x01000100 },
    { 0x02000002, 0x00200000, 0x00020000, 0x00000000, 0x00000000, 0x00001000, 0x00000100, 0x10000010 },
    { 0x20000000, 0x02000000, 0x00200000, 0x00020000, 0x00001000, 0x00000100, 0x00000010, 0x00000001 },
    { 0x00000000, 0x20000000, 0x02000000, 0x00201000, 0x00020100, 0x00000010, 0x00000001, 0x00000000 },
    { 0x00000000, 0x00000000, 0x20001000, 0x02000100, 0x00200010, 0x00020001, 0x00000000, 0x00000000 },
    { 0x00000000, 0x00001000, 0x00000100, 0x20000010, 0x02000001, 0x00200000, 0x00020000, 0x00000000 },
    { 0x00001000, 0x00000100, 0x00000010, 0x00000001, 0x20000000, 0x02000000, 0x00200000, 0x00020000 },
    { 0x00040100, 0x00000010, 0x00000001, 0x00000000, 0x00000000, 0x20000000, 0x02000000, 0x00200800 },
    { 0x00400010, 0x00040001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x20000800, 0x02000080 },
    { 0x04000001, 0x00400000, 0x00040000, 0x00000000, 0x00000000, 0x00000800, 0x00000080, 0x20000008 },
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 }
};

uint32_t ucode[8];
uint32_t ucode_scrambled[8];

int dw2bit[8][32];
int dw2dw[8][32];

int main(int argc, char *argv[])
{
    unsigned int bit, dw, i, b;
    uint32_t mask, andmask;

    if (argc < 2) {
        printf("need a MS ROM dump input file!\n");
        exit(EXIT_FAILURE);
    }

    for (dw = 0; dw < 8; dw++) {
        for (bit = 0; bit < 32; bit++) {
            dw2bit[dw][bit] = -1;
            dw2dw[dw][bit] = -1;
        }
    }

    for (dw = 0; dw < 8; dw++) {
        for (bit = 0; bit < 32; bit++) {
            mask = 1UL << bit;
            for (i = 0; i < 8; i++) {
                andmask = dw_to_crbusrom[bit][dw] & dw_masks[i][dw];
                if (andmask) {
                    int bitpos = __builtin_ffs(andmask) - 1;
                    assert(dw2bit[dw][bitpos] == -1);
                    assert(dw2dw[dw][bitpos] == -1);
                    dw2bit[dw][bitpos] = bit;
                    dw2dw[dw][bitpos] = i;
                }
            }
        }
    }

#if 0
    printf("MSROM to SCRAMBLED:\n");
    for (dw = 0; dw < 8; dw++) {
        for (bit = 0; bit < 32; bit++) {
            int b = dw2dw[dw][bit] * 32 + dw2bit[dw][bit];
            printf("[%d] -> [", dw * 32 + bit);
            if (b > 0) {
                printf("%d]\n", b);
            } else {
                printf("NC]\n");
            }
        }
    }
#endif
    for (dw = 0; dw < 8; dw++) {
        for (bit = 0; bit < 32; bit++) {
            if (ucode[dw] & (1UL << bit)) {
                ucode_scrambled[dw2dw[dw][bit]] |=
                    (1UL << dw2bit[dw][bit]);
            }
        }
    }

    char *ts;
    FILE *file;
    int addr, raddr;
    int g;
    uint32_t *groupbase;
    char line_buf[4096];


    file = fopen(argv[1], "r");
    if (!file) {
        perror("Could not open MSROM dump input file");
        exit(EXIT_FAILURE);
    }

    while (fgets(line_buf, sizeof line_buf, file)) {
        ts = strtok(line_buf, ": ");
        if (!ts)
            continue;
        addr = strtol(ts, NULL, 16);
        if (addr % 8) {
            fprintf(stderr, "Misaligned address in input :%08X\n", addr);
            exit(EXIT_FAILURE);
        }

        for (g = 0; g < 8; g++) {
            ts = strtok(NULL, " ");
            if (!ts) {
                fprintf(stderr, "Incomplete data for address %04X", raddr);
                exit(EXIT_FAILURE);
            }
            ucode[g] = strtol(ts, NULL, 16);
            ucode_scrambled[g] = 0;
        }

        for (dw = 0; dw < 8; dw++) {
            for (bit = 0; bit < 32; bit++) {
                if (ucode[dw] & (1UL << bit)) {
                    ucode_scrambled[dw2dw[dw][bit]] |= (1UL << dw2bit[dw][bit]);
                }
            }
        }

        printf("%04x:", addr);
        for (g = 0; g < 8; g++) {
            printf(" %08x", ucode_scrambled[g]);
        }
        printf("\n");
    }
    fclose(file);
}
