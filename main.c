//Uses Jo Engine, copyright Johannes Fetz
//MIT
/**
In Loving Memory of Jeffrey Neil Lindamood,
my father. 1962 - 2019.
And Bonnie K Caulder Lindamood,
my grandmother. 1937 - 2019.
I am sorry for the pain you had to go through.
**/
//
// Compilation updated to use latest version of Jo Engine standard compiler.
//
//
#include <sl_def.h>
#include "def.h"
#include <SEGA_INT.H>
#include <SEGA_GFS.H>

//Outstanding code contributions from XL2 
//

#include "mymath.h"
#include "render.h"
#include "collision.h"
#include "control.h"
#include "hmap.h"
#include "vdp2.h"
#include "physobjet.h"
#include "tga.h"
#include "ldata.h"
#include "input.h"
#include "pcmsys.h"
#include "pcmstm.h"
#include "draw.h"
#include "anidefs.h"
#include "gamespeed.h"
#include "menu.h"
#include "sound.h"
#include "particle.h"
//
#include "lwram.c"
//
//
#include "dspm.h"
//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
// SGL Work Area is using the last 200KB of High Work RAM. The game binary is using about 250KB.
unsigned char hwram_model_data[HWRAM_MODEL_DATA_HEAP_SIZE];
void * HWRAM_ldptr;
void * HWRAM_hldptr;
//


/*

1.2 To-Do List

1 - check
2 - check 
3 - check
4 - check, different solution applied
5 - check
6 - i just made it transparent, should be enough

8 - check, just a name is fine
9 - check
10 - different solution applied
11 - check
12 - check

1. Patch level 2 to fix the backwards rotated overhang and the ungettable ring							
2. Add option to invert camera
3. Adjust the camera when jumping and holding UP to look further forward
	- Need to base this on Y input control angle, not just UP (because analog)
4. Start the smart camera lockout timer when holding DOWN, stop it when you press UP (but only after DOWN)
	- Need to base this on Y input control angle
5. Holding A will only trigger slide when on the ground / or otherwise decouple slide camera from holding A
6. Shrink / move / adjust the flag so it isn't in your face when you have it

7. Try to push the camera where you are facing a little more past your turn when you are turning

8. Add a name / image to each level in menu
9. Add an indication that the reset point has been successfully set (sound effect + event)
10. Stop the movement smart cam when no buttons are pressed
11. Fix up the "Greece" assets to reduce Z-fighting
12. fix the strafe cam

*/


//
short * division_table;

//short * sine_table;

//A zero vector to be used when you want zero.
POINT zPt = {0, 0, 0};
extern Sint8 SynchConst; //SGL System Variable

unsigned char * dirty_buf;
void * currentAddress;

volatile Uint32 * scuireg = (Uint32*)0x25FE00A4;
volatile Uint32 * scuimask = (Uint32*)0x25FE00A0;
volatile Uint32 * scudmareg =  (Uint32*)0x25FE007C;

int game_set_res = TV_320x240;

//////////////////////////////////////////////////////////////////////////////
int flagIconTexno = 0;
//////////////////////////////////////////////////////////////////////////////
// Player data struct
//////////////////////////////////////////////////////////////////////////////
	_player you;

//Loading. Check msfs.c and mloader c/h
void	load_test(void)
{

	get_file_in_memory((Sint8*)"NBG_PAL.TGA", (void*)dirty_buf);
	set_tga_to_nbg1_palette((void*)dirty_buf);
	
	Uint32 fid = GFS_NameToId((Sint8*)"SPLASH_4.TGA");
	set_8bpp_tga_to_nbg0_image(fid, dirty_buf);
	
	WRAP_NewPalette((Sint8*)"TADA.TGA", (void*)dirty_buf);
	baseAsciiTexno = numTex;
	sprAsciiHeight = 12;
	sprAsciiWidth = WRAP_NewTable((Sint8*)"FONT2.TGA", dirty_buf, sprAsciiHeight); //last argument, tex height
	//
	HWRAM_ldptr = (void *)(&hwram_model_data[0]);
	////////////////////////////////////////////////
	// REMINDER: All file names must comply with the 8.3 standard.
	// File extensions can be no longer than 3 letters.
	// File names can be no longer than 8 letters.
	// The total length is thusly 12 characters (as there is a period).
	////////////////////////////////////////////////
	nbg_sprintf(0,0, "Loading ...");
	//stmsnd[stm_win] = (Sint8*)"WIN.ADX";
	//stmsnd[stm_freturn] = (Sint8*)"FRETURN.ADX";
	//stmsnd[stm_orchit0] = (Sint8*)"ORCHIT0.ADX";
	snd_win = load_adx((Sint8*)"WIN.ADX");
	snd_yeah = load_adx((Sint8*)"YEAH.ADX");
	snd_freturn = load_adx((Sint8*)"FRETURN.ADX");
	snd_orchit0 = load_adx((Sint8*)"ORCHIT0.ADX");
	snd_ftake = load_adx((Sint8*)"FLAG.ADX");
	snd_tslow = load_adx((Sint8*)"TSLOW.ADX");
	snd_gpass = load_adx((Sint8*)"GPASS.ADX");
	snd_rlap = load_adx((Sint8*)"RLAP.ADX");
	snd_bwee = load_8bit_pcm((Sint8*)"BWEE.PCM", 15360);
	snd_lstep = load_8bit_pcm((Sint8*)"LSTEP.PCM", 15360);
	snd_mstep = load_8bit_pcm((Sint8*)"MSTEP.PCM", 15360);
	snd_slideon = load_8bit_pcm((Sint8*)"SLIDEON.PCM", 15360);
	snd_wind = load_8bit_pcm((Sint8*)"WND.PCM", 15360);
	snd_bstep = load_8bit_pcm((Sint8*)"STEP.PCM", 15360);
	snd_cronch = load_8bit_pcm((Sint8*)"CRONCH.PCM", 15360);
	snd_alarm = load_8bit_pcm((Sint8*)"ALARM.PCM", 15360);
	snd_smack = load_8bit_pcm((Sint8*)"MSMACK.PCM", 15360);
	snd_boost = load_8bit_pcm((Sint8*)"BOOST.PCM", 15360);
	snd_khit = load_8bit_pcm((Sint8*)"KICKHIT.PCM", 7680);
	snd_clack = load_8bit_pcm((Sint8*)"CLACK.PCM", 7680);
	snd_click = load_8bit_pcm((Sint8*)"CLICK.PCM", 7680);
	snd_close = load_8bit_pcm((Sint8*)"CLOSE.PCM", 7680);
	snd_button = load_8bit_pcm((Sint8*)"BUTTON1.PCM", 7680);
	snd_button2 = load_8bit_pcm((Sint8*)"BUTTON2.PCM", 7680);
	snd_ffield1 = load_8bit_pcm((Sint8*)"FOPEN.PCM", 7680);
	snd_ffield2 = load_8bit_pcm((Sint8*)"FCLOSE.PCM", 7680);
	snd_ring1 = load_8bit_pcm((Sint8*)"CRING1.PCM", 7680);
	snd_ring2 = load_8bit_pcm((Sint8*)"CRING2.PCM", 7680);
	snd_ring3 = load_8bit_pcm((Sint8*)"CRING3.PCM", 7680);
	snd_ring4 = load_8bit_pcm((Sint8*)"CRING4.PCM", 7680);
	snd_ring5 = load_8bit_pcm((Sint8*)"CRING5.PCM", 7680);
	snd_ring6 = load_8bit_pcm((Sint8*)"CRING6.PCM", 7680);
	snd_ring7 = load_8bit_pcm((Sint8*)"CRING7.PCM", 7680);
	snd_flagflap = load_8bit_pcm((Sint8*)"FLAGFLAP.PCM", 15360);
	baseRingMenuTexno = numTex;
	WRAP_NewTable((Sint8*)"RINGNUM.TGA", (void*)dirty_buf, 0);
	flagIconTexno = numTex;
	WRAP_NewTexture((Sint8*)"FLAGICON.TGA", (void*)dirty_buf);
	sparkTexno = numTex;
	WRAP_NewTexture((Sint8*)"SPARK.TGA", (void*)dirty_buf);
	auraTexno = numTex;
	WRAP_NewTexture((Sint8*)"AURA.TGA", (void*)dirty_buf);
	puffTexno = numTex;
	WRAP_NewTexture((Sint8*)"PUFF.TGA", (void*)dirty_buf);
	controlImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"CONTROL2.TGA", (void*)dirty_buf);
	slowImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"SLOW.TGA", (void*)dirty_buf);
	parImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"PAR.TGA", (void*)dirty_buf);
	goldImgTexno = numTex;
	WRAP_NewTexture((Sint8*)"GOLD.TGA", (void*)dirty_buf);

	animated_texture_list[0] = numTex;
	WRAP_NewTable((Sint8*)"QMARK.TGA", (void*)dirty_buf, 16);
	animated_texture_list[1] = numTex;
	WRAP_NewTable((Sint8*)"ARROW.TGA", (void*)dirty_buf, 24);  
	animated_texture_list[2] = numTex;
	WRAP_NewTable((Sint8*)"CHECK.TGA", (void*)dirty_buf, 16);  
	animated_texture_list[3] = numTex;
	WRAP_NewTable((Sint8*)"GOAL.TGA", (void*)dirty_buf, 16);  
	animated_texture_list[4] = numTex;
	WRAP_NewTable((Sint8*)"LEYEANIM.TGA", (void*)dirty_buf, 24);  
	animated_texture_list[5] = numTex;
	WRAP_NewTable((Sint8*)"REYEANIM.TGA", (void*)dirty_buf, 24);  
	animated_texture_list[6] = numTex;
	WRAP_NewTable((Sint8*)"ARROW2.TGA", (void*)dirty_buf, 24);  
	animated_texture_list[7] = numTex;
	WRAP_NewTable((Sint8*)"ARROW3.TGA", (void*)dirty_buf, 24);  
	animated_texture_list[8] = numTex;
	WRAP_NewTable((Sint8*)"FFIELD.TGA", (void*)dirty_buf, 24);  
	
	/////////////////////////////////////
	// Floor / heightmap textures
	/////////////////////////////////////
	map_texture_table_numbers[0] = numTex;
	WRAP_NewTable((Sint8*)&map_tex_tbl_names[0], (void*)dirty_buf, 0);
	map_texture_table_numbers[1] = numTex;
	WRAP_NewTable((Sint8*)&map_tex_tbl_names[1], (void*)dirty_buf, 0);
	map_texture_table_numbers[2] = numTex;
	WRAP_NewTable((Sint8*)&map_tex_tbl_names[2], (void*)dirty_buf, 0);
	map_texture_table_numbers[3] = numTex;
	WRAP_NewTable((Sint8*)&map_tex_tbl_names[3], (void*)dirty_buf, 0);
	map_texture_table_numbers[4] = numTex;
	WRAP_NewTable((Sint8*)&map_tex_tbl_names[4], (void*)dirty_buf, 0);
	map_end_of_original_textures = numTex;
	map_tex_amt = (numTex - map_texture_table_numbers[0]);
	make_4way_combined_textures(map_texture_table_numbers[0], map_end_of_original_textures, 0);
	map_last_combined_texno = numTex;
	make_dithered_textures_for_map(0);

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PONY.GVP", 		HWRAM_ldptr, &pl_model,    GV_SORT_CEN, MODEL_TYPE_PLAYER, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"WINGS.GVP", 		HWRAM_ldptr, &wings,	    GV_SORT_CEN, MODEL_TYPE_PLAYER, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SHADOW.GVP", 		HWRAM_ldptr, &shadow,	    GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"MARKER.GVP",		HWRAM_ldptr, &entities[8], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING1.GVP",		HWRAM_ldptr, &entities[1], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING2.GVP",		HWRAM_ldptr, &entities[2], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING3.GVP",		HWRAM_ldptr, &entities[3], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING4.GVP",		HWRAM_ldptr, &entities[4], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING5.GVP",		HWRAM_ldptr, &entities[5], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING6.GVP",		HWRAM_ldptr, &entities[6], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RING7.GVP",		HWRAM_ldptr, &entities[7], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"MARKER.GVP",		HWRAM_ldptr, &entities[8], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"KYOOB.GVP",		HWRAM_ldptr, &entities[9], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PLATF00.GVP",		HWRAM_ldptr, &entities[10], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLAG.GVP",			HWRAM_ldptr, &entities[57], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL); 
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SIGN.GVP",			HWRAM_ldptr, &entities[36], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"ARBOX.GVP",		HWRAM_ldptr, &entities[37], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BARBOX.GVP",		HWRAM_ldptr, &entities[38], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BOOSTER.GVP",		HWRAM_ldptr, &entities[46], GV_SORT_CEN, MODEL_TYPE_NORMAL, NULL);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TEST00.GVP",		HWRAM_ldptr, &entities[0], GV_SORT_CEN, MODEL_TYPE_TPACK, NULL);
		
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FFIELD.GVP",		HWRAM_ldptr, &entities[55], GV_SORT_CEN, MODEL_TYPE_NORMAL, &entities[0]); 
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BRIDGE1.GVP",		HWRAM_ldptr, &entities[11], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE01.GVP",		HWRAM_ldptr, &entities[12], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE02.GVP",		HWRAM_ldptr, &entities[13], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE03.GVP",		HWRAM_ldptr, &entities[14], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GREECE04.GVP",		HWRAM_ldptr, &entities[15], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"OVRHNG.GVP",		HWRAM_ldptr, &entities[16], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"PIER1.GVP",		HWRAM_ldptr, &entities[17], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"POST00.GVP",		HWRAM_ldptr, &entities[18], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TUNNEL2.GVP",		HWRAM_ldptr, &entities[19], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TUNNEL3.GVP",		HWRAM_ldptr, &entities[20], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"WALL1.GVP",		HWRAM_ldptr, &entities[21], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT03.GVP",		HWRAM_ldptr, &entities[22], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BRIDGE2.GVP",		HWRAM_ldptr, &entities[23], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"OBSTCL1.GVP",		HWRAM_ldptr, &entities[24], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT01.GVP",		HWRAM_ldptr, &entities[25], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLOAT02.GVP",		HWRAM_ldptr, &entities[26], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY01.GVP",		HWRAM_ldptr, &entities[27], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY02.GVP",		HWRAM_ldptr, &entities[28], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY03.GVP",		HWRAM_ldptr, &entities[29], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY04.GVP",		HWRAM_ldptr, &entities[30], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY05.GVP",		HWRAM_ldptr, &entities[31], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY06.GVP",		HWRAM_ldptr, &entities[32], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"RAMP01.GVP",		HWRAM_ldptr, &entities[33], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"HIWAY07.GVP",		HWRAM_ldptr, &entities[34], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TOWER01.GVP",		HWRAM_ldptr, &entities[35], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"POST01.GVP",		HWRAM_ldptr, &entities[39], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"POST02.GVP",		HWRAM_ldptr, &entities[40], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);

	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"BRIDGE0.GVP",		HWRAM_ldptr, &entities[41], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"MMAPB.GVP",		HWRAM_ldptr, &entities[42], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"LONGBOX.GVP",		HWRAM_ldptr, &entities[43], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"TUTI.GVP",			HWRAM_ldptr, &entities[44], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"CLIMTOW.GVP",		HWRAM_ldptr, &entities[45], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"SATRAMP.GVP",		HWRAM_ldptr, &entities[47], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"G_PLANE.GVP",		HWRAM_ldptr, &entities[50], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"STARSTAN.GVP",		HWRAM_ldptr, &entities[51], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"FLAGSTAN.GVP",		HWRAM_ldptr, &entities[53], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_ldptr = gvLoad3Dmodel((Sint8*)"GOALSTAN.GVP",		HWRAM_ldptr, &entities[54], GV_SORT_CEN, MODEL_TYPE_BUILDING, &entities[0]);
	HWRAM_hldptr = HWRAM_ldptr;
	
	p64MapRequest(8);
	//
	
}

void	game_frame(void)
{
	
	// nbg_sprintf(2, 9, "ax(%i)", apd1.ax);
	// nbg_sprintf(2, 10, "ay(%i)", apd1.ay);
	// nbg_sprintf(2, 11, "lt(%i)", apd1.lta);
	// nbg_sprintf(2, 12, "rt(%i)", apd1.rta);

	slCashPurge();
	prep_map_mtx();
	
	update_gamespeed();
	maintRand();
	master_draw_stats();
	frame_render_prep();
	master_draw(); 
	operate_stage_music();
	reset_pad(&pad1);

	//ABC+Start Exit Condition
	if(is_key_down(DIGI_START) && is_key_down(DIGI_A) && is_key_down(DIGI_B) && is_key_down(DIGI_C))
	{
		SYS_Exit(0);
	}

}

void	my_vlank(void)
{
	vblank_requirements();
	operate_digital_pad1();
	//Sound Driver Stuff
	sdrv_stm_vblank_rq();
	sdrv_vblank_rq();
	//
}

void	attributions(void)
{
	slPrint("Created by Ponut64", slLocate(3, 4));
	slPrint("Contributions:", slLocate(3, 6));
	slPrint("XL2 - Essential knowledge & tools", slLocate(3, 7));
	//slPrint("dannyduarte - fixed workarea.c", slLocate(3, 8));
	slPrint("ReyeMe - for his czechnology", slLocate(3, 9));
	slPrint("Emerald Nova - fixed-point timer", slLocate(3, 11));
	slPrint("fafling - actually read the manuals", slLocate(3, 13));
	slPrint("mrkotftw - formal programmer guy", slLocate(3, 14));
	slPrint("Johannez Fetz - good example code", slLocate(3, 15));
	
	slPrint("Music __STOLEN__ from:", slLocate(3,16));
	slPrint("Red Vox, Freedom Planet, Pizza Tower", slLocate(3,17));
	slPrint("Also: SGAP (Captive Unicorns)", slLocate(3, 18));
	slPrint("Also: AndTheRainfall",	slLocate(3, 19));
	slPrint("Also: Whowever made Shazfunk", slLocate(3, 20));

	slPrint("Sound Driver by Ponut64", slLocate(3, 22));
	slPrint("Great Value Game Engine", slLocate(3, 23));
	slPrint("also by Ponut64", slLocate(3, 24));
	
	//testing_level_data((Sint8*)"LIST0.REM", (void*)dirty_buf);
	slZoomNbg0(65536, 65536);
	load_test();
	
	nbg_clear_text();
	//For NBG0, we zoom it.
	//This zooms the screen such that a 128x128 area is displayed over the 352x224 screen.
	//This is for th zoom on in-game backgrounds. Before this point, the zoom is set for the splash screen.
	slZoomNbg0(23831, 37449);
}

int		validation_escape(void)
{
	static int first_stroke = 0;
	static int escaped = 0;
	if(is_key_pressed(DIGI_L) && is_key_pressed(DIGI_R))
	{
		first_stroke = 1;
	}
	
	if(first_stroke == 1 && is_key_pressed(DIGI_DOWN) && is_key_pressed(DIGI_START) && is_key_up(DIGI_L) && is_key_up(DIGI_R))
	{
		escaped = 1;
	}
	return escaped;
}

void	hardware_validation(void)
{
	load_drv(ADX_MASTER_1536); 
	load_hmap_prog();
	sdrv_vblank_rq();
	update_gamespeed();
	int start_time = get_time_in_frame();
	run_hmap_prog(); //Dry-run the DSP to get it to flag done
	
 	while(dsp_noti_addr[0] == 0){
		if(validation_escape()) break;
	}; //"DSP Wait"
	
	int time_at_end = get_time_in_frame();
	
	while(m68k_com->start != 0)
	{
		if(validation_escape()) break;
	}; //68K Wait
	
	int time_at_sound = get_time_in_frame();
	

	
	if((time_at_end - start_time) < 111411)
	{
	get_file_in_memory((Sint8*)"NBG_PAL.TGA", (void*)dirty_buf);
	set_tga_to_nbg1_palette((void*)dirty_buf);
		while(1)
		{
			slPrintFX(time_at_end - start_time, slLocate(4, 6));
			slPrintFX(time_at_sound - start_time, slLocate(4, 8));
			slBack1ColSet((void*)back_scrn_colr_addr, 0x801F);
			nbg_sprintf(5, 10, "There is something wrong with");
			nbg_sprintf(5, 11, "your Saturn video game system.");
			nbg_sprintf(5, 12, "You can contact our associates");
			nbg_sprintf(5, 13, "Jane Mednafen or John Bizhawk");
			nbg_sprintf(5, 14, "for the necessary repairs.");
			if(validation_escape()) break;
		}
	}
	
	if(((time_at_sound - start_time) > (50<<16)))
	{
	get_file_in_memory((Sint8*)"NBG_PAL.TGA", (void*)dirty_buf);
	set_tga_to_nbg1_palette((void*)dirty_buf);
		while(1)
		{
			slBack1ColSet((void*)back_scrn_colr_addr, 0x9B26);
			nbg_sprintf(1, 10, "Listen, I know you are using an emulator.");
			nbg_sprintf(1, 11, "That, or a PAL / modded Saturn.");
			nbg_sprintf(1, 12, "My point is Saturn emulation is flawed.");
			nbg_sprintf(1, 13, "Mednafen/Bizhawk are pretty good.");
			nbg_sprintf(1, 14, "Kronos/SSF are almost good.");
			nbg_sprintf(1, 15, "But please be aware:");
			nbg_sprintf(1, 16, "It's not the ideal experience.");
			nbg_sprintf(1, 17, "Press START to continue!");
			if(is_key_pressed(DIGI_START)) break;
		}
	}
	 nbg_clear_text();
}

int	main(void)
{
	//jo_core_init(0xE726); 
	game_set_res = (hi_res_switch) ? TV_704x448 : TV_352x224;
	slInitSystem(game_set_res, NULL, 2);
	slDynamicFrame(ON); //Dynamic framerate
    SynchConst = (Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
	//
	//Loading Area
	//
	init_lwram();
	init_vdp2(0xE726);
	init_render_area(90 * 182);
	initPhys();
	init_box_handling();
	init_heightmap();
	//The one interrupt that SGL has you register
	slIntFunction(my_vlank);
	//
	//
	init_dsp_programs();
	//Sound Driver & Hardware/Emulator Validation
	hardware_validation();
	//
	
	fill_obj_list();
	init_entity_list();
	
	//load_test();
	attributions();
	init_particle();
	init_hud_events();
	
	set_camera();
	reset_player();
	anim_defs();
	add_adx_front_buffer(15360);
	add_adx_back_buffer(dirty_buf);
	pcm_stream_init(30720, PCM_TYPE_8BIT);
	pcm_stream_host(game_frame);
	return 1;
}

