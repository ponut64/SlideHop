//
//control.h 
//include for use of control functions
#pragma once

#define MAX_SPEED_FACTOR	(300)
#define STICK_MAX			(122)

//Can also add deadzone and allow/unallow analog lock-out here.
typedef struct {
	int followForce;
	int cameraAccel;
	int cameraCap;
	int movementCam;
	int facingCam;
	int lockoutTime;
	int lockTimer;
	int lockout;
	int invert;
} _controlOptions;

extern _controlOptions usrCntrlOption;

extern int spdfactr;

extern int reval;
extern int target;

extern Bool holdCam;

void	controls(void);


