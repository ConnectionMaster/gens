#ifndef GENS_DEBUGGER_H
#define GENS_DEBUGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUGGER

extern int adr_mem;
extern int nb_inst;
extern int pattern_pal;
extern int pattern_adr;
extern int Current_PC;

void Debug_Event(int key, int mod);
void Update_Debug_Screen(void);

enum DEBUG_MODE
{
	DEBUG_NONE		= 0,
	DEBUG_MAIN_68000	= 1,
	DEBUG_Z80		= 2,
	DEBUG_GENESIS_VDP	= 3,
	DEBUG_SUB_68000_REG	= 4,
	DEBUG_SUB_68000_CDC	= 5,
	DEBUG_WORD_RAM_PATTERN	= 6,
	DEBUG_MAIN_SH2		= 7,
	DEBUG_SUB_SH2		= 8,
	DEBUG_32X_VDP		= 9,
};

#endif /* GENS_DEBUGGER */

#ifdef __cplusplus
}
#endif

#endif /* GENS_DEBUG_H */
