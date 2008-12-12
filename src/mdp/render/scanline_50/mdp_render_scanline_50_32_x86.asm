;
; Gens: [MDP] 50% Scanline renderer. [32-bit color] (x86 asm version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
;
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 2 of the License, or (at your
; option) any later version.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;

arg_destScreen	equ 28+8
arg_mdScreen	equ 28+12
arg_destPitch	equ 28+16
arg_srcPitch	equ 28+20
arg_width	equ 28+24
arg_height	equ 28+28

MASK_DIV2_32	equ 0x7F7F7F7F

; Symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_mdp_render_scanline_50_32_x86		mdp_render_scanline_50_32_x86
	%define	_mdp_render_scanline_50_32_x86_mmx	mdp_render_scanline_50_32_x86_mmx
%endif

; Position-independent code macros.
%include "pic.inc"

; Read-only data on Win32 uses the section name ".rdata".
%ifdef __OBJ_WIN32
	%define .rodata .rdata
%endif

section .rodata align=64
	
	; 64-bit mask used for the MMX version.
	MASK_DIV2_32_MMX:	dd 0x7F7F7F7F, 0x7F7F7F7F
	
section .text align=64
	
	align 64
	
	;************************************************************************
	; void mdp_render_scanline_50_32_x86(uint32_t *destScreen, uint32_t *mdScreen,
	;				     int destPitch, int srcPitch,
	;				     int width, int height);
	global _mdp_render_scanline_50_32_x86
	_mdp_render_scanline_50_32_x86:
		
		; Save registers.
		pushad
		
		mov	ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
		mov	esi, [esp + arg_mdScreen]	; esi = Source
		shl	ecx, 2
		sub	[esp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebp, ecx			; ebp = Difference between dest pitch and src pitch
		mov	edi, [esp + arg_destScreen]	; edi = Destination
		shr	ecx, 4				; Transfer 16 bytes per cycle. (4 32-bit pixels)
		mov	[esp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X1:
				; Get source pixels.
				mov	eax, [esi]	; First pixel.
				mov	edx, [esi + 4]	; Second pixel.
				add	esi, 8
				
				; Put destination pixels.
				mov	[edi], eax
				mov	[edi + 4], eax
				mov	[edi + 8] , edx
				mov	[edi + 12], edx
				add	edi, 16
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X1
			
			mov	ecx, [esp + arg_width]	; ecx = Number of pixels per line
			add	edi, ebp		; Add the destination pitch difference.
			shl	ecx, 3
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 3
			jmp	short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; First pixel.
				mov	eax, [esi]
				shr	eax, 1			; 50% scanline
				and	eax, MASK_DIV2_32	; Mask off the MSB of each color component.
				mov	[edi], eax
				mov	[edi + 4], eax
				
				; Second pixel.
				mov	eax, [esi + 4]
				shr	eax, 1			; 50% scanline
				and	eax, MASK_DIV2_32	; Mask off the MSB of each color component.
				mov	[edi + 8], eax
				mov	[edi + 12], eax
				
				; Next group of pixels.
				add	esi, 8
				add	edi, 16
				dec	ecx
				jnz	short .Loop_X2
			
			add	esi, [esp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebp			; Add the destination pitch difference.
			mov	ecx, [esp + arg_width]		; Reset the X counter.
			dec	dword [esp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Restore registers.
		popad
		ret
	
	align 64
	
	;************************************************************************
	; void mdp_render_scanline_50_32_x86_mmx(uint32_t *destScreen, uint32_t *mdScreen,
	;					 int destPitch, int srcPitch,
	;					 int width, int height);
	global _mdp_render_scanline_50_32_x86_mmx
	_mdp_render_scanline_50_32_x86_mmx:
		
		; Save registers.
		pushad
		
		; (PIC) Get the Global Offset Table.
		get_GOT
		
		mov	ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
		mov	esi, [esp + arg_mdScreen]	; esi = Source
		shl	ecx, 2
		sub	[esp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebp, ecx			; ebp = Difference between dest pitch and src pitch
		mov	edi, [esp + arg_destScreen]	; edi = Destination
		shr	ecx, 5				; Transfer 32 bytes per cycle. (8 32-bit pixels)
		mov	[esp + arg_width], ecx		; Initialize the X counter.
		get_movq_localvar mm7, MASK_DIV2_32_MMX	; Load the mask.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X1:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm1, mm0
				movq	mm2, [esi + 8]
				movq	mm3, mm2
				add	esi, byte 16
				
				; Unpack source pixels.
				punpckldq	mm1, mm1
				punpckhdq	mm0, mm0
				punpckldq	mm3, mm3
				punpckhdq	mm2, mm2
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				add	edi, byte 32
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X1
			
			mov	ecx, [esp + arg_width]	; Reset the X counter.
			add	edi, ebp		; Add the destination pitch difference.
			shl	ecx, 4
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 4
			jmp	short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm2, [esi + 8]
				add	esi, byte 16
				
				; 50% scanline.
				psrlq	mm0, 1
				psrlq	mm2, 1
				pand	mm0, mm7
				pand	mm2, mm7
				
				; Unpack pixels.
				movq	mm1, mm0
				movq	mm3, mm2
				punpckldq	mm1, mm1
				punpckhdq	mm0, mm0
				punpckldq	mm3, mm3
				punpckhdq	mm2, mm2
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				add	edi, byte 32
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X2
			
			add	esi, [esp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebp			; Add the destination pitch difference.
			mov	ecx, [esp + arg_width]		; Reset the X counter.
			dec	dword [esp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Restore registers.
		emms
		popad
		ret
