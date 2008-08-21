/**
 * GENS: Input handler.
 */

#ifndef G_INPUT_H
#define G_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

// OS-specific includes.
#if (defined(__linux__))
#include "g_input_linux.h"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif


// Controller keymapping.
struct K_Def
{
	unsigned int Start, Mode;
	unsigned int A, B, C;
	unsigned int X, Y, Z;
	unsigned int Up, Down, Left, Right;
};
extern unsigned char Kaillera_Keys[16];
extern struct K_Def Keys_Def[8];


void Input_KeyDown(int key, int mod);
void Input_KeyUp(int key, int mod);

#ifdef __cplusplus
}
#endif

#endif
