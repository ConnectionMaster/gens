/**
 * GENS: Main loop. (Linux-specific code)
 */

#ifndef G_MAIN_LINUX_H
#define G_MAIN_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL.h>
#include "g_input.h"

void SetWindowText(const char *text);
void SetWindowVisibility (int visibility);

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);
int Init_OS_Graphics(void);
void End_OS_Graphics(void);

// TODO: This isn't OS-independent...
void update_SDL_events(void);

#ifdef __cplusplus
}
#endif

#endif
