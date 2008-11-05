/**
 * Gens: Video Drawing class - SDL + OpenGL
 */

#ifndef GENS_V_DRAW_SDL_GL_HPP
#define GENS_V_DRAW_SDL_GL_HPP

#include "v_draw_sdl.hpp"

// OpenGL includes
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

// Needed for VSync on Linux.
// Code copied from Mesa's glxswapcontrol.c
// TODO: Make sure this works correctly on all drivers.

#ifndef GLX_MESA_swap_control
typedef GLint (*PFNGLXSWAPINTERVALMESAPROC)(unsigned interval);
typedef GLint (*PFNGLXGETSWAPINTERVALMESAPROC)( void );
#endif

#if !defined(GLX_OML_sync_control) && defined(_STDINT_H)
#define GLX_OML_sync_control 1
typedef Bool (*PFNGLXGETMSCRATEOMLPROC)(Display *dpy, GLXDrawable drawable, int32_t *numerator, int32_t *denominator);
#endif

#ifndef GLX_MESA_swap_frame_usage
#define GLX_MESA_swap_frame_usage 1
typedef int (*PFNGLXGETFRAMEUSAGEMESAPROC)(Display *dpy, GLXDrawable drawable, float * usage);
#endif

class VDraw_SDL_GL : public VDraw_SDL
{
	public:
		VDraw_SDL_GL();
		VDraw_SDL_GL(VDraw *oldDraw);
		~VDraw_SDL_GL();
		
		int Init_Video(void);
		void End_Video(void);
		
		// Clear the screen.
		void clearScreen(void);
		//void Clear_Primary_Screen(void);
		//void Clear_Back_Screen(void);
		
		// Update VSync value.
		void updateVSync(bool fromInitSDLGL = false);
		
	protected:
		int initRenderer(const int w, const int h, const bool reinitSDL = true);
		
		// Flip the screen buffer. (Called by v_draw.)
		void flipInternal(void);
		
		// Adjust stretch parameters.
		void stretchAdjustInternal(void);
		
		// Update the renderer.
		void updateRenderer(void);
		
		// SDL flags
		static const int SDL_GL_Flags =
				SDL_DOUBLEBUF |
				SDL_HWSURFACE |
				SDL_HWPALETTE |
				SDL_ASYNCBLIT |
				SDL_HWACCEL |
				SDL_OPENGL;
		
		// GL variables.
		GLuint textures[1];
		int rowLength;
		int textureSize;
		unsigned char *filterBuffer;
		int filterBufferSize;
		unsigned int m_pixelFormat;
		unsigned int m_pixelType;
		
		// Stretch parameters
		float m_HStretch, m_VStretch;
		
		// OpenGL VSync stuff
		// Copied from Mesa's glxswapcontrol.c
		PFNGLXSWAPINTERVALMESAPROC set_swap_interval;
		PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval;
};

#endif
