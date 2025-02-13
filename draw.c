
#include <sl_def.h>
#include <SGL.H>
#include <SEGA_GFS.H>

#include "def.h"
#include "mymath.h"
#include "input.h"
#include "control.h"
#include "vdp2.h"
#include "render.h"
#include "tga.h"
#include "physobjet.h"
#include "hmap.h"
#include "collision.h"
#include "pcmstm.h"
#include "menu.h"
#include "particle.h"
#include "gamespeed.h"
//
#include "dspm.h"
//
#include "height.h"

#include "draw.h"

FIXED sun_light[3] = {5000, -20000, 0};
//Player Model
entity_t pl_model;
//Player's Shadow
entity_t shadow;
//Player Wings
entity_t wings;
//Heightmap Matrix
MATRIX hmap_mtx;
//Root matrix after perspective transform
MATRIX perspective_root;
// ???
MATRIX unit;
//Billboard sprite work list (world-space positions)
//
// Mental note: Try Rotation-16 framebuffer and alternate screen coordinate from 0,0 and 1,1 every vblank
// Should make mesh transparencies work better

FIXED hmap_matrix_pos[XYZ] = {0, 0, 0};
FIXED hmap_actual_pos[XYZ] = {0, 0, 0};

//Note: global light is in order of BLUE, GREEN, RED. [BGR]
int globalColorOffset;
int glblLightApply = true;
int drawModeSwitch = DRAW_MASTER;
unsigned char * backScrn = (unsigned char *)VDP2_RAMBASE;

//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//////////////////////////////////////////////////////////////////////////////
animationControl idle;
animationControl idleB;
animationControl stop;
animationControl fall;
animationControl slideIdle;
animationControl slideLln;
animationControl slideRln;
animationControl airIdle;
animationControl airLeft;
animationControl airRight;
animationControl jump;
animationControl hop;
animationControl walk;
animationControl run;
animationControl dbound;
animationControl climbIdle;
animationControl climbing;
 
animationControl flap;

spriteAnimation qmark;
spriteAnimation arrow;
spriteAnimation check;
spriteAnimation goal;
spriteAnimation LeyeAnim;
spriteAnimation ReyeAnim;
spriteAnimation arrow2;
spriteAnimation arrow3;
spriteAnimation field;

void	computeLight(void)
{

	if(glblLightApply == true)
	{
	
	color_offset_vdp1_palette(globalColorOffset, &glblLightApply);
	//Next, set the sun light.
	active_lights[0].pop = 1;
	active_lights[0].ambient_light = &sun_light[0];
	active_lights[0].min_bright = 10000;
	active_lights[0].bright = 0;
	//////////////////////////////////////////////////////////////////////////////////////

	glblLightApply = false;
	}
}

void	set_camera(void)
{
 POINT viewPnt = {0, 0, 1<<16};	

	slLookAt( viewPnt , zPt , 0 );
}

void	master_draw_stats(void)
{
		if(viewInfoTxt == 1)
		{
	slPrintFX(you.pos[X], slLocate(9, 1));
	slPrintFX(you.pos[Y], slLocate(19, 1));
	slPrintFX(you.pos[Z], slLocate(29, 1));

	//nbg_sprintf(18, 0, "(File System Status)");
	nbg_sprintf(27, 2, "Pts :%x:", you.points);
	//nbg_sprintf(10, 2, "throttle:(%i)", you.IPaccel);
	//slPrintFX(you.sanics, slLocate(26, 3));
//		if(delta_time>>6 > 35)
//		{
	nbg_sprintf(8, 25, "TRPLY:                ");
	nbg_sprintf(8, 25, "TRPLY:%i", transPolys[0]);
	nbg_sprintf(8, 26, "SNTPL:                ");
	nbg_sprintf(8, 26, "SNTPL:%i", ssh2SentPolys[0] + msh2SentPolys[0]);
	nbg_sprintf(8, 27, "VERTS:                ");
	nbg_sprintf(8, 27, "VERTS:%i", transVerts[0]);
//		}
	nbg_sprintf(37, 26, "cX(%i)", you.cellPos[X]);
	nbg_sprintf(37, 27, "cY(%i)", you.cellPos[Y]);    
	
	nbg_sprintf(1, 4, "Fuel:(%i), Rate:(%i)", you.power, you.IPaccel);
	
	nbg_sprintf(16, 2, "Stream:(%i)", file_system_status_reporting);
	nbg_sprintf(17, 3, "Sanics:(%i)", you.sanics);
	
	
		} else if(viewInfoTxt == 2)
		{
			
			slPrintFX(you.pos[X], slLocate(9, 1));
			slPrintFX(you.pos[Y], slLocate(19, 1));
			slPrintFX(you.pos[Z], slLocate(29, 1));
			
			slPrintFX(you.velocity[X], slLocate(9, 2));
			slPrintFX(you.velocity[Y], slLocate(19, 2));
			slPrintFX(you.velocity[Z], slLocate(29, 2));
			
			slPrintFX(you.dV[X], slLocate(9, 3));
			slPrintFX(you.dV[Y], slLocate(19, 3));
			slPrintFX(you.dV[Z], slLocate(29, 3));
			
			slPrintFX(you.ControlUV[X], slLocate(9, 4));
			slPrintFX(you.ControlUV[Y], slLocate(19, 4));
			slPrintFX(you.ControlUV[Z], slLocate(29, 4));
			
			nbg_sprintf(2, 5, "rX:(%i)", you.rot[X]);
			nbg_sprintf(15, 5, "rY:(%i)", you.rot[Y]);
			nbg_sprintf(29, 5, "rZ:(%i)", you.rot[Z]);
			
			nbg_sprintf(2, 6, "rRX:(%i)", you.renderRot[X]);
			nbg_sprintf(15, 6,"rRY:(%i)", you.renderRot[Y]);
			nbg_sprintf(29, 6, "rRZ:(%i)", you.renderRot[Z]);
			
			nbg_sprintf(2, 7, "hitWall:(%i)", you.hitWall);
			nbg_sprintf(2, 8, "hitSurf:(%i)", you.hitSurface);
			nbg_sprintf(20, 8,"hitItm:(%i)", pl_RBB.collisionID);
			
		}
}

void	player_animation(void)
{
	
			//Animation Chains
					static int airTimer = 0;
			if(pl_model.file_done == true)
			{
				if(you.hitSurface == true)
				{
				airTimer = 0;
					if(you.setSlide != true && you.climbing != true && airTimer == 0)
					{
						if(you.velocity[X] == 0 && you.velocity[Y] == 0 && you.velocity[Z] == 0)
						{
							meshAnimProcessing(&idle, &pl_model,  false);
						} else if( (you.velocity[X] != 0 || you.velocity[Z] != 0) && you.dirInp)
						{
						if(you.IPaccel <= 0){
								meshAnimProcessing(&stop, &pl_model,  false);
							}
						if(you.sanics < 2<<16 && you.IPaccel > 0){
								meshAnimProcessing(&walk, &pl_model,  true);
							}
						if(you.sanics < 3<<16 && you.sanics > 2<<16){
								meshAnimProcessing(&run, &pl_model,  true);
							}
						if(you.sanics >= 3<<16){
								meshAnimProcessing(&dbound, &pl_model,  true);
							}
						} else if((you.velocity[X] != 0 || you.velocity[Z] != 0) && !you.dirInp)
						{
							meshAnimProcessing(&stop, &pl_model,  false);
						} else {
							meshAnimProcessing(&idle, &pl_model,  false);
						}	
						//IF NOT SLIDE ENDIF
					} else if(you.setSlide == true && you.climbing != true){
						if(you.rot2[Y] > (30 * 182) && you.rot2[Y]  < (150 * 182) && you.dirInp)
						{
							meshAnimProcessing(&slideRln, &pl_model,  false);
						} else if(you.rot2[Y] < (330 * 182) && you.rot2[Y] > (210 * 182) && you.dirInp)
						{
							meshAnimProcessing(&slideLln, &pl_model,  false);
						} else {
							meshAnimProcessing(&slideIdle, &pl_model,  false);
						}
						//IF SLIDE ENDIF
					} else if(you.climbing == true)
					{
						if(you.sanics == 0)
						{
							meshAnimProcessing(&climbIdle, &pl_model,  false);
						} else {
							meshAnimProcessing(&climbing, &pl_model,  false);
						}
						//IF CLIMB ENDIF
					}
					//IF SURFACE ENDIF	
				} else {
						airTimer++;
						if(airTimer < 8 && airTimer != 0 && you.velocity[Y] != 0)
						{
							if(!you.setJet){
								meshAnimProcessing(&jump, &pl_model,  false);
							} else {
								meshAnimProcessing(&hop, &pl_model,  false);
							}
						} else if(you.rot2[Y] > (30 * 182) && you.rot2[Y]  < (150 * 182) && you.dirInp)
						{
							meshAnimProcessing(&airRight, &pl_model,  false);
						} else if(you.rot2[Y] < (330 * 182) && you.rot2[Y] > (210 * 182) && you.dirInp)
						{
							meshAnimProcessing(&airLeft, &pl_model,  false);
						} else {
							meshAnimProcessing(&airIdle, &pl_model,  false);
						}
				}//IF AIR ENDIF
			} //IF MODEL LOADED ENDIF
	
			//This plays a wing flap animation when you start 'jetting'.
			//Due to the configuration of the animation, the wings stop after one flap.
			//This code manually resets the flap animation when you're done jetting, so they will flap again.
			if(you.setJet)
			{
				meshAnimProcessing(&flap, &wings,  false);
				//Flap when hitting surface.
				if(you.hitWall)
				{
					flap.curFrm = flap.startFrm * 8;
					flap.curKeyFrm = flap.startFrm;
					flap.reset_enable = 'Y';
				}
			} else {
				flap.curFrm = flap.startFrm * 8;
				flap.curKeyFrm = flap.startFrm;
				flap.reset_enable = 'Y';
			}
	
}

void	player_draw(int draw_mode)
{
	

	//Note that "sl_RBB" is used as a safe copy of pl_RBB to be manipulated.
	sl_RBB = pl_RBB;
	sl_RBB.pos[X] = 0;//-sl_RBB.pos[X]; //Negate, because coordinate madness
	sl_RBB.pos[Y] = 0;//-sl_RBB.pos[Y]; //Negate, because coordinate madness
	sl_RBB.pos[Z] = 0;//-sl_RBB.pos[Z]; //Negate, because coordinate madness
	
	pl_model.prematrix = (FIXED*)&sl_RBB;
	wings.prematrix = (FIXED*)&sl_RBB;
	if(draw_mode == DRAW_MASTER)
	{
		msh2DrawModel(&pl_model, perspective_root);
	} else if(draw_mode == DRAW_SLAVE)
	{
		slPushMatrix();
		ssh2DrawModel(&pl_model);
		slPopMatrix();
	}
	
	if(you.setJet)
	{
		if(draw_mode == DRAW_MASTER)
		{
			msh2DrawModel(&wings, perspective_root);
		} else if(draw_mode == DRAW_SLAVE)
		{
			slPushMatrix();
			ssh2DrawModel(&wings);
			slPopMatrix();
		}
	}

}

void	obj_draw_queue(void)
{
		
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++)
	{
		//This conditions covers if somehow a non-renderable object (like level data) got put into the render stack.
		//Assuming the rest of the game code made sense up to this point. Else the game's gonna crash here.
		if(DBBs[i].status[0] != 'R') continue;
		if(DBBs[i].status[4] == 'S') apply_box_scale(&DBBs[i]);
		//unsigned short objType = (dWorldObjects[activeObjects[i]].type.ext_dat & ETYPE);
		DBBs[i].status[5] = 'r';
	slPushMatrix();
	
		entities[objDRAW[i]].prematrix = (FIXED *)&DBBs[i];
	
		if(entities[objDRAW[i]].type == MODEL_TYPE_BUILDING)
		{ 
			plane_rendering_with_subdivision(&entities[objDRAW[i]]);
		} else {
			
			ssh2DrawModel(&entities[objDRAW[i]]);
		}
	slPopMatrix();
	
	}
	

	for(int s = 0; s < MAX_SPRITES; s++)
	{
		if(sprWorkList[s].lifetime >= 0)
		{
			sprWorkList[s].lifetime -= delta_time;
			if(sprWorkList[s].type == SPRITE_TYPE_BILLBOARD || sprWorkList[s].type == SPRITE_TYPE_UNSCALED_BILLBOARD)
			{
				ssh2BillboardScaledSprite(&sprWorkList[s]);
			} else if(sprWorkList[s].type == SPRITE_TYPE_3DLINE || sprWorkList[s].type == SPRITE_TYPE_UNSORTED_LINE)
			{
				ssh2Line(&sprWorkList[s]);
			} else if(sprWorkList[s].type == SPRITE_TYPE_NORMAL || sprWorkList[s].type == SPRITE_MESH_STROBE
			|| sprWorkList[s].type == SPRITE_FLASH_STROBE || sprWorkList[s].type == SPRITE_BLINK_STROBE)
			{
				ssh2NormalSprite(&sprWorkList[s]);
			}
		} else {
			//Mark expired sprites as unused.
			sprWorkList[s].type = 'N'; 
		}
	}
	
}

void	shadow_draw(int draw_mode)
{
	
	//Make shadow match player rotation. I mean, it's not a perfect solution, but it mostly works.
	//Note that "sl_RBB" is used as a slave-only copy of pl_RBB to be manipulated.
	sl_RBB = pl_RBB;
	sl_RBB.pos[X] = you.pos[X] - you.shadowPos[X];
	sl_RBB.pos[Y] = you.pos[Y] - (you.shadowPos[Y] - 8192);
	sl_RBB.pos[Z] = you.pos[Z] - you.shadowPos[Z]; 
	
	shadow.prematrix = (FIXED*)&sl_RBB;

	if(draw_mode == DRAW_MASTER)
	{
		msh2DrawModel(&shadow, perspective_root);
	} else if(draw_mode == DRAW_SLAVE)
	{
		slPushMatrix();
		ssh2DrawModel(&shadow);
		slPopMatrix();
	}
	
}

 //Uses SGL to prepare the matrix for the map, so it doesn't mess up the matrix stack when the map draws
 //Be aware the location of this function is important:
 //The player's position/rotation cannot change from between when it runs and when the matrix is used.
void	prep_map_mtx(void)
{
	slInitMatrix();
	set_camera();
	slPushMatrix();
	{
	slTranslate((VIEW_OFFSET_X), (VIEW_OFFSET_Y), (VIEW_OFFSET_Z) );
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	slTranslate(hmap_matrix_pos[X], you.pos[Y], hmap_matrix_pos[Z]);
	slGetMatrix(hmap_mtx);
	}
	slPopMatrix();
}

	//volatile int times[8];

void	object_draw(void)
{
	/////////////////////////////////////////////
	// Important first-step
	// Sometimes the master will finish preparing the drawing list while the slave is working on them.
	// That would normally break the work flow and make some entities disappear.
	// To prevent that, we capture the to-draw lists at the start of the Slave's workflow to a list that the Master does not edit.
	/////////////////////////////////////////////
	for(int i = 0; i < objNEW; i++)
	{
		objDRAW[i] = objPREP[i];
	}
	for(int i = 0; i < MAX_PHYS_PROXY; i++)
	{
		DBBs[i] = RBBs[i];
	}
	*timeComm = 0;

	computeLight();
	slPushMatrix();
	{	
	slTranslate((VIEW_OFFSET_X), (VIEW_OFFSET_Y), (VIEW_OFFSET_Z) );
	
	//Take care about the order of the matrix transformations!
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	slGetMatrix(perspective_root);
	if(drawModeSwitch == DRAW_MASTER)
	{
		player_draw(DRAW_SLAVE);
		shadow_draw(DRAW_SLAVE);
	}
	//////////////////////////////////////////////////////////////
	// "viewpoint" is the point from which the perspective will originate (contains view translation/rotation).
	//////////////////////////////////////////////////////////////
	you.viewpoint[X] = fxm(perspective_root[X][X], perspective_root[3][X]) +
						fxm(perspective_root[Y][X], perspective_root[3][Y]) +
						fxm(perspective_root[Z][X], perspective_root[3][Z]);
						
	you.viewpoint[Y] = fxm(perspective_root[X][Y], perspective_root[3][X]) +
						fxm(perspective_root[Y][Y], perspective_root[3][Y]) +
						fxm(perspective_root[Z][Y], perspective_root[3][Z]);
						
	you.viewpoint[Z] = fxm(perspective_root[X][Z], perspective_root[3][X]) +
						fxm(perspective_root[Y][Z], perspective_root[3][Y]) +
						fxm(perspective_root[Z][Z], perspective_root[3][Z]);
		//
	slTranslate(you.pos[X], you.pos[Y], you.pos[Z]);

	obj_draw_queue();

	}
	slPopMatrix();
	
}

void	map_draw_prep(void)
{
	hmap_cluster();
	
	//Loads the DSP pepperbox
	you.prevCellPos[X] = you.cellPos[X];
	you.prevCellPos[Y] = you.cellPos[Y];
	you.prevDispPos[X] = you.dispPos[X];
	you.prevDispPos[Y] = you.dispPos[Y];
	//View Distance Extention -- Makes turning view cause performance issue, beware?
	you.cellPos[X] = (fxm((INV_CELL_SIZE), you.pos[X])>>16);
	you.cellPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z])>>16);
	int center_distance = (CELL_SIZE_INT * ((LCL_MAP_PLY>>1)-1))<<16;
	int sineY = fxm(slSin(-you.viewRot[Y]), center_distance);
	int sineX = fxm(slCos(-you.viewRot[Y]), center_distance);
	you.dispPos[X] = (fxm((INV_CELL_SIZE), you.pos[X] +  sineY)>>16);
	you.dispPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z] +  sineX)>>16);
	//
	hmap_matrix_pos[X] = (you.pos[X] + you.velocity[X]) - ((you.dispPos[X] * CELL_SIZE_INT)<<16);
	hmap_matrix_pos[Z] = (you.pos[Z] + you.velocity[Z]) - ((you.dispPos[Y] * CELL_SIZE_INT)<<16);
	
	hmap_actual_pos[X] = hmap_matrix_pos[X] - (you.pos[X] + you.velocity[X]);
	hmap_actual_pos[Y] = 0;
	hmap_actual_pos[Z] = hmap_matrix_pos[Z] - (you.pos[Z] + you.velocity[Z]);
	
	load_hmap_prog();
	run_hmap_prog();

}

void	map_draw(void)
{	

	while(dsp_noti_addr[0] == 0){}; //"DSP Wait"
	update_hmap(hmap_mtx);

}

void	background_draw(void)
{
	
	//
	// What do I want to do now with the background layer?
	// It is 512x512. I want the screen to move to appear as if it is matching rotation on the Y axis.
	// I also want Y-0 rotation to display the center of the background image.
	// First: How much can the screen span in the X axis for this to be possible?
	// So with 512 pixels as the width, one degree is 1.4222 pixels. We could abstract that so 128 units is 1 pixel.
	// (128 being 1 pixel is achieved by 65536 (360 degrees) / 512 pixels)
	
	// The X rotation - Y screen axis is completely different however.
	// The Y axis of the image will NOT rollover. It would rollover in space, but we aren't in space.
	// It also is not a spherical projection or even a logical projection; I can't invert the image when it rolls over.
	// There are ways to manipulate the image data itself such that rollovers are masked, but I will instead limit the rotation.
	// The view rotation here will be limited to +/- 90 degrees.
	// 16384 is the integer value of 90 degrees, but we allow a range of +/- 90, so our total degree range is 32768.
	// That means the unit value of 1 pixel is (32768 units / 512 pixels) = 64 units per pixel
	// But we end up using 85 units per pixel. For... reasons, I guess. It's what worked. No clear idea why.
	int vrx = you.viewRot[X];
	
	int screen_y_to_x = 192 - ((you.viewRot[Y] / 128));
	int screen_x_to_y = 192 - ((vrx / 85));
	// Screen Rollover Control
	// When the Y rotation has caused the screen to rollover (e.g. 192 + y_to_x > 512), the image is offset.
	// So we need to get a position that corresponds to a position under 512.
	if(JO_ABS(screen_y_to_x) > 512)
	{
		screen_y_to_x = screen_y_to_x % 512;
	}

	slScrPosNbg0(screen_y_to_x<<16, screen_x_to_y<<16);
	
	//Trying to draw something to represent the sanics
	//Micro-project. Trying to heal brain.
	//nbg_sprintf(1,20, "         ");
	//nbg_sprintf(1,20, "%i", you.sanics);
	int x0l = 8;
	int yax = 160;
	int x1l = 64;
	for(int i = 0; i < 8; i++)
	{
		draw_hud_line(x0l, yax+i, x1l, yax+i, 7); //Black/Gray
	}
	//Scale sanics to 0-56
	int sscl = fxdiv(you.sanics, 15<<16);
	if(sscl >= 1<<16) sscl = 1<<16;
	if(sscl < 0) sscl = 0;
	x1l = fxm(sscl, 56<<16)>>16;
	if(x1l < 0) x1l = 0;
	x1l += 8;
	for(int i = 0; i < 8; i++)
	{
		draw_hud_line(x0l, yax+i, x1l, yax+i, 6); //Blue
	}
}

void	master_draw(void)
{
	static int time_at_start;
	static int time_of_master_draw;
	static int time_of_object_management;
	static int time_at_end;
	static int time_at_ssh2_end;
	static int interim_time;
	static int extra_time;

	time_at_start = get_time_in_frame();

	drawModeSwitch = (you.distanceToMapFloor < 768<<16) ? DRAW_MASTER : DRAW_SLAVE;

	if(!you.inMenu)
	{
		
	slSlaveFunc(object_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();

	interim_time = get_time_in_frame();
	
	background_draw();
	player_animation();

	//
	if(drawModeSwitch == DRAW_MASTER)
	{
	map_draw();
	} else {
	player_draw(DRAW_MASTER);
	shadow_draw(DRAW_MASTER);
	}
	map_draw_prep();
	//
	time_of_master_draw = get_time_in_frame() - interim_time;
	interim_time = get_time_in_frame();
	//
	operate_particles();
	hud_menu();

	//
	
		//No Touch Order -- Affects animations/mechanics
		controls();
		player_phys_affect();
		player_collision_test_loop();
		collide_with_heightmap(&pl_RBB, &you.fwd_world_faces, &you.time_axis);
		//
	extra_time = get_time_in_frame() - interim_time;
	
	interim_time = get_time_in_frame();
	flush_boxes(0);
	light_control_loop(); //lit
	object_control_loop(you.dispPos);
	time_of_object_management = get_time_in_frame() - interim_time;
	slSlaveFunc(sort_master_polys, 0);
		//
	} else if(you.inMenu)
	{
		start_menu();
		//
		slSlaveFunc(sort_master_polys, 0);
		//
	}
	
	//Oh, right, this...
	//I was intending on integrating some sort of process-based loop on these.
	//Oh well...
	clean_sprite_animations();
	start_texture_animation(&check, &entities[18]);
	start_texture_animation(&qmark, &entities[36]);
	start_texture_animation(&arrow, &entities[37]);
	start_texture_animation(&arrow, &entities[38]);
	start_texture_animation(&arrow, &entities[39]);
	start_texture_animation(&goal, &entities[40]);
	start_texture_animation(&LeyeAnim, &pl_model);
	start_texture_animation(&ReyeAnim, &pl_model);
	start_texture_animation(&arrow2, &entities[46]);
	start_texture_animation(&arrow3, &entities[46]);
	start_texture_animation(&field, &entities[55]);
	operate_texture_animations();

	time_at_end = get_time_in_frame();
	//Debug stuff. Important!
	
	if(viewInfoTxt == 1)
	{
	nbg_sprintf_decimal(7, 7, time_at_start);
	nbg_sprintf_decimal(7, 8, time_of_master_draw);
	nbg_sprintf_decimal(7, 9, time_of_object_management);
	nbg_sprintf_decimal(7, 10, extra_time);
	nbg_sprintf_decimal(7, 11, time_at_end);
	nbg_sprintf(2, 6, "MSH2:");
	nbg_sprintf(2, 7, "Strt:");
	nbg_sprintf(2, 8, "Map:");
	nbg_sprintf(2, 9, "Objs:");
	nbg_sprintf(2, 10, "Ext:");
	nbg_sprintf(2, 11, "End:");
	
	while(!*timeComm){
		if(get_time_in_frame() >= (50<<16)) break;
	};
	time_at_ssh2_end = get_time_in_frame();
	nbg_sprintf_decimal(7, 12, time_at_ssh2_end);
	nbg_sprintf(2, 12, "SSH2:");
	}
}

