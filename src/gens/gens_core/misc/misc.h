#ifndef GENS_MISC_H
#define GENS_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#define EMU_MODE 0x01
#define BLANC    0x00
#define BLEU     0x02
#define VERT     0x04
#define ROUGE    0x06
#define TRANS    0x08
#define SIZE_X2  0x10

void Print_Text_asm(const char *str, int Size, int Pos_X, int Pos_Y, int Style);

void CDD_Export_Status(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MISC_H */
