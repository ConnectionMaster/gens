/**
 * GENS: Sega CD (Mega CD) initialization and main loop code.
 */

#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "g_main.hpp"
#include "g_update.hpp"
#include "sdllayer/g_sdlsound.h"
#include "sdllayer/g_sdlinput.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/cpu/z80/z80.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/io/io.h"
#include "util/file/save.h"
#include "util/file/ggenie.h"
#include "segacd/cd_sys.hpp"
#include "segacd/lc89510.h"
#include "gens_core/gfx/gfx_cd.h"
#include "util/sound/wave.h"
#include "gens_core/sound/pcm.h"
#include "segacd/cd_sys.hpp"
#include "segacd/cd_file.h"
#include "util/sound/gym.h"

#include "gens_core/misc/misc.h"
#include "gens_core/misc/byteswap.h"

#include "ui-common.h"


unsigned char CD_Data[GENS_PATH_MAX];	// Used for hard reset to know the game name

/**
 * Detect_Country_SegaCD(): Detect the country code of a SegaCD game.
 * @return BIOS file for the required country.
 */
char* Detect_Country_SegaCD(void)
{
	if (CD_Data[0x10B] == 0x64)
	{
		Game_Mode = 1;
		CPU_Mode = 1;
		return BIOS_Filenames.MegaCD_EU;
	}
	else if (CD_Data[0x10B] == 0xA1)
	{
		Game_Mode = 0;
		CPU_Mode = 0;
		return BIOS_Filenames.MegaCD_JP;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
		return BIOS_Filenames.SegaCD_US;
	}
}

/**
 * Init_SegaCD(): Initialize the Sega CD with the specified ISO image.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful; 0 if an error occurred.
 */
int Init_SegaCD(const char *iso_name)
{
	char Str_Err[256], *Bios_To_Use;
	
	UI_Set_Window_Title_Init("SegaCD", 0);
	
	if (Reset_CD((char*)CD_Data, iso_name))
	{
		// Error occured while setting up Sega CD emulation.
		// TODO: Show a message box.
		UI_Set_Window_Title_Idle();
		return 0;
	}
	
	// Check what BIOS should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			Bios_To_Use = Detect_Country_SegaCD ();
			break;
		
		case 0: // Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			Bios_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
		
		case 1: // US (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			Bios_To_Use = BIOS_Filenames.MegaCD_EU;
			break;
		
		case 2: // Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			Bios_To_Use = BIOS_Filenames.MegaCD_EU;
			break;
		
		case 3: // Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			Bios_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
	}
	
	// Attempt to load the Sega CD BIOS.
	if (Load_SegaCD_BIOS(Bios_To_Use) == NULL)
	{
		UI_MsgBox("Your Sega CD BIOS files aren't configured correctly.\nGo to menu 'Options -> BIOS/Misc Files' to set them up.",
			  "BIOS Configuration Error");
		UI_Set_Window_Title_Idle();
		return 0;
	}
	
	Update_CD_Rom_Name((char*)&CD_Data[32]);
	
	// Set the window title to the localized console name and the game name.
	if ((CPU_Mode == 1) || (Game_Mode == 0))
		strcpy(Str_Err, "MegaCD");
	else
		strcpy(Str_Err, "SegaCD");
	UI_Set_Window_Title_Game(Str_Err, Rom_Name);
	
	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	BRAM_Ex_State &= 0x100;
	Controller_1_COM = Controller_2_COM = 0;
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	Set_Clock_Freq(1);
	
	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data, Rom_Size);
	
	// Reset all CPUs and other components.
	M68K_Reset (2);
	S68K_Reset ();
	Z80_Reset ();
	Reset_VDP ();
	LC89510_Reset ();
	Init_RS_GFX ();
	
	Init_PCM (Sound_Rate);
	
	// Initialize sound.
	if (Sound_Enable)
	{
		End_Sound ();
		
		if (!Init_Sound ())
			Sound_Enable = 0;
		else
			Play_Sound ();
	}
	
	Load_BRAM();			// Initialise BRAM
	Load_Patch_File();		// Only used to reset Patch structure
	
	Reset_Update_Timers();
	
	if (SegaCD_Accurate)
	{
		Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}
	else
	{
		Update_Frame = Do_SegaCD_Frame;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}
	
	return 1;
}


/**
 * Reload_SegaCD(): Reloads the Sega CD.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful.
 */
int Reload_SegaCD(const char *iso_name)
{
	char Str_Err[256];
	
	Save_BRAM ();
	
	UI_Set_Window_Title_Init("SegaCD", 1);
	
	Reset_CD ((char *) CD_Data, iso_name);
	Update_CD_Rom_Name ((char *) &CD_Data[32]);
	
	// Set the window title to the localized console name and the game name.
	if ((CPU_Mode == 1) || (Game_Mode == 0))
		strcpy(Str_Err, "MegaCD");
	else
		strcpy(Str_Err, "SegaCD");
	UI_Set_Window_Title_Game(Str_Err, Rom_Name);
	
	Load_BRAM();
	
	return 1;
}


/**
 * Reset_SegaCD(): Resets the Sega CD.
 */
void
Reset_SegaCD ()
{
	char *Bios_To_Use;
	
	if (CPU_Mode)
		Bios_To_Use = BIOS_Filenames.MegaCD_EU;
	else if (Game_Mode)
		Bios_To_Use = BIOS_Filenames.SegaCD_US;
	else
		Bios_To_Use = BIOS_Filenames.MegaCD_JP;
	
	//SetCurrentDirectory(Gens_Path);
	
	if (Detect_Format (Bios_To_Use) == -1)
	{
		UI_MsgBox(
			"Your Sega CD BIOS files aren't configured correctly.\n"
			"Go to menu 'Options -> BIOS/Misc Files' to set them up.",
			"BIOS Configuration Error"
			);
		return;
	}
	
	Controller_1_COM = Controller_2_COM = 0;
	SRAM_ON = 0;
	SRAM_Write = 0;
	Paused = 0;
	BRAM_Ex_State &= 0x100;
	
	// Load the BIOS file.
	Game = Load_ROM(Bios_To_Use, 0);
	
	Update_CD_Rom_Name ((char *) &CD_Data[32]);
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data, Rom_Size);
	
	// Reset all CPUs and other components.
	M68K_Reset (2);
	S68K_Reset ();
	Z80_Reset ();
	LC89510_Reset ();
	Reset_VDP ();
	Init_RS_GFX ();
	Reset_PCM ();
	YM2612_Reset ();
	
	if (CPU_Mode)
		VDP_Status |= 1;
	else
		VDP_Status &= ~1;
}


/**
 * Do_SegaCD_Frame_No_VDP(): Runs a Sega CD frame without updating the VDP.
 * @return 1 if successful.
 */
int Do_SegaCD_Frame_No_VDP (void)
{
	int *buf[2];
	int HInt_Counter;
	
	// Set the number of visible lines.
	SET_VISIBLE_LINES;

	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer ();
	sub68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable)
		Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert ();
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (DMAT_Length)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec (Cycles_M68K - 360);
	sub68k_exec (Cycles_S68K - 586);
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	z80_Interrupt (&M_Z80, 0xFF);
	
	main68k_exec (Cycles_M68K);
	sub68k_exec (Cycles_S68K);
	Z80_EXEC(0);
	
	Update_SegaCD_Timer ();
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	Update_CD_Audio (buf, Seg_Length);
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	return 1;
}


/**
 * Do_SegaCD_Frame_No_VDP_Cycle_Accurate(): Runs a cycle-accurate Sega CD frame without updating the VDP.
 * @return 1 if successful.
 */
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate (void)
{
	int *buf[2], i, j;
	int HInt_Counter;
	
	// Set the number of visible lines.
	SET_VISIBLE_LINES;
	
	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer ();
	sub68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec (i);
			i += 24;
			
			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K - 404);
		sub68k_exec (Cycles_S68K - 658);
		
		/* end instruction by instruction execution */
		
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
			i += 24;
			
			if (j < Cycles_S68K)
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		
		/* end instruction by instruction execution */
		
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable)
		Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert ();
	
	i = Cycles_M68K + 24;
	j = Cycles_S68K + 39;
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (DMAT_Length)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	
	/* instruction by instruction execution */
	
	while (i < (Cycles_M68K - 360))
	{
		main68k_exec (i);
		i += 24;
		
		if (j < (Cycles_S68K - 586))
		{
			sub68k_exec (j);
			j += 39;
		}
	}
	
	main68k_exec (Cycles_M68K - 360);
	sub68k_exec (Cycles_S68K - 586);
	
	/* end instruction by instruction execution */
	
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	z80_Interrupt (&M_Z80, 0xFF);
	
	/* instruction by instruction execution */
	
	while (i < Cycles_M68K)
	{
		main68k_exec (i);
		i += 24;
		
		if (j < Cycles_S68K)
		{
			sub68k_exec (j);
			j += 39;
		}
	}
	
	main68k_exec (Cycles_M68K);
	sub68k_exec (Cycles_S68K);
	
	/* end instruction by instruction execution */
	
	Z80_EXEC(0);
	
	Update_SegaCD_Timer ();
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
             VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec (i);
			i += 24;
			
			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K - 404);
		sub68k_exec (Cycles_S68K - 658);
		
		/* end instruction by instruction execution */
		
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
			i += 24;
			
			if (j < Cycles_S68K)
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		
		/* end instruction by instruction execution */
		
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	Update_CD_Audio (buf, Seg_Length);
	
  	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	return 1;
}


/**
 * SegaCD_Display_LED(): Display the LEDs on the Sega CD interface.
 */
inline void SegaCD_Display_LED()
{
	// TODO: Optimize this function.
	
	const unsigned short ledReady15 = 0x03E0;
	const unsigned short ledReady16 = 0x07C0;
	unsigned short ledReady;
	
	const unsigned short ledAccess15 = 0x7C00;
	const unsigned short ledAccess16 = 0xF800;
	unsigned short ledAccess;
	
	const unsigned int ledReady32 = 0x00F800;
	const unsigned int ledAccess32 = 0xF80000;
	
	const unsigned int ledRow1 = 336 * 220;
	const unsigned int ledRow2 = 336 * 222;
	
	if (bpp == 15)
	{
		ledReady = ledReady15;
		ledAccess = ledAccess15;
	}
	else
	{
		ledReady = ledReady16;
		ledAccess = ledAccess16;
	}
	
	// Ready LED
	if (LED_Status & 2)
	{
		MD_Screen[ledRow1 + 12] = ledReady;
		MD_Screen[ledRow1 + 13] = ledReady;
		MD_Screen[ledRow1 + 14] = ledReady;
		MD_Screen[ledRow1 + 15] = ledReady;
		MD_Screen[ledRow2 + 12] = ledReady;
		MD_Screen[ledRow2 + 13] = ledReady;
		MD_Screen[ledRow2 + 14] = ledReady;
		MD_Screen[ledRow2 + 15] = ledReady;
		
		MD_Screen32[ledRow1 + 12] = ledReady32;
		MD_Screen32[ledRow1 + 13] = ledReady32;
		MD_Screen32[ledRow1 + 14] = ledReady32;
		MD_Screen32[ledRow1 + 15] = ledReady32;
		MD_Screen32[ledRow2 + 12] = ledReady32;
		MD_Screen32[ledRow2 + 13] = ledReady32;
		MD_Screen32[ledRow2 + 14] = ledReady32;
		MD_Screen32[ledRow2 + 15] = ledReady32;
	}
	
	// Access LED
	if (LED_Status & 1)
	{
		MD_Screen[ledRow1 + 12 + 8] = ledAccess;
		MD_Screen[ledRow1 + 13 + 8] = ledAccess;
		MD_Screen[ledRow1 + 14 + 8] = ledAccess;
		MD_Screen[ledRow1 + 15 + 8] = ledAccess;
		MD_Screen[ledRow2 + 12 + 8] = ledAccess;
		MD_Screen[ledRow2 + 13 + 8] = ledAccess;
		MD_Screen[ledRow2 + 14 + 8] = ledAccess;
		MD_Screen[ledRow2 + 15 + 8] = ledAccess;
		
		MD_Screen32[ledRow1 + 12 + 8] = ledAccess32;
		MD_Screen32[ledRow1 + 13 + 8] = ledAccess32;
		MD_Screen32[ledRow1 + 14 + 8] = ledAccess32;
		MD_Screen32[ledRow1 + 15 + 8] = ledAccess32;
		MD_Screen32[ledRow2 + 12 + 8] = ledAccess32;
		MD_Screen32[ledRow2 + 13 + 8] = ledAccess32;
		MD_Screen32[ledRow2 + 14 + 8] = ledAccess32;
		MD_Screen32[ledRow2 + 15 + 8] = ledAccess32;
	}
}


/**
 * Do_SegaCD_Frame(): Runs a Sega CD frame.
 * @return 1 if successful.
 */
int Do_SegaCD_Frame(void)
{
	int *buf[2];
	int HInt_Counter;
	
	// Set the number of visible lines.
	SET_VISIBLE_LINES;
	
	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer ();
	sub68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		Render_Line ();
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable)
		Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert ();
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (DMAT_Length)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec (Cycles_M68K - 360);
	sub68k_exec (Cycles_S68K - 586);
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	z80_Interrupt (&M_Z80, 0xFF);
	
	main68k_exec (Cycles_M68K);
	sub68k_exec (Cycles_S68K);
	Z80_EXEC(0);
	
	Update_SegaCD_Timer ();
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	Update_CD_Audio (buf, Seg_Length);
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	if (Show_LED)
		SegaCD_Display_LED();
	
	return 1;
}


/**
 * Do_SegaCD_Frame_Cycle_Accurate(): Runs a cycle-accurate Sega CD frame.
 * @return 1 if successful.
 */
int Do_SegaCD_Frame_Cycle_Accurate(void)
{
	int *buf[2], i, j;
	int HInt_Counter;
	
	// Set the number of visible lines.
	SET_VISIBLE_LINES;
	
	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer ();
	sub68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec (i);
			i += 24;
			
			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K - 404);
		sub68k_exec (Cycles_S68K - 658);
		
		/* end instruction by instruction execution */
		
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		Render_Line ();
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
			i += 24;
			
			if (j < Cycles_S68K)
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		
		/* end instruction by instruction execution */
		
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable)
		Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert ();
	
	i = Cycles_M68K + 24;
	j = Cycles_S68K + 39;
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (DMAT_Length)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	
	/* instruction by instruction execution */
	
	while (i < (Cycles_M68K - 360))
	{
		main68k_exec (i);
		i += 24;
		
		if (j < (Cycles_S68K - 586))
		{
			sub68k_exec (j);
			j += 39;
		}
	}
	
	main68k_exec (Cycles_M68K - 360);
	sub68k_exec (Cycles_S68K - 586);
	
	/* end instruction by instruction execution */
	
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	z80_Interrupt (&M_Z80, 0xFF);
	
	/* instruction by instruction execution */
	
	while (i < Cycles_M68K)
	{
		main68k_exec (i);
		i += 24;
		
		if (j < Cycles_S68K)
		{
			sub68k_exec (j);
			j += 39;
		}
	}
	
	main68k_exec (Cycles_M68K);
	sub68k_exec (Cycles_S68K);
	
	/* end instruction by instruction execution */
	
	Z80_EXEC(0);
	
	Update_SegaCD_Timer ();
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert ();
		
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec (i);
			i += 24;
			
			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K - 404);
		sub68k_exec (Cycles_S68K - 658);
		
		/* end instruction by instruction execution */
		
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
			i += 24;		// Chuck Rock intro need faster timing ... strange.
			
			if (j < Cycles_S68K)
			{
				sub68k_exec (j);
				j += 39;
			}
		}
		
		main68k_exec (Cycles_M68K);
		sub68k_exec (Cycles_S68K);
		
		/* end instruction by instruction execution */
		
		Z80_EXEC(0);
		
		Update_SegaCD_Timer ();
	}
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	Update_CD_Audio (buf, Seg_Length);
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	if (Show_LED)
		SegaCD_Display_LED();
	
	return 1;
}
