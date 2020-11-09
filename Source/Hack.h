#pragma once

#include <windows.h>
#include <cstdlib>

#define _USE_MATH_DEFINES

#ifndef POW
#define POW(x) ((x)*(x))
#endif

#include <math.h>

#include "GameSDK/wrect.h"
#include "GameSDK/cl_dll.h"
#include "GameSDK/interface.h"
#include "GameSDK/r_studioint.h"
#include "GameSDK/cl_entity.h"
#include "GameSDK/con_nprint.h"
#include "GameSDK/weapons.h"
#include "GameSDK/pmtrace.h"
#include "GameSDK/pm_defs.h"
#include "GameSDK/weapons.h"

#include "Offset.h"
#include "detours.h"

#pragma warning(disable:4244)

class cHack
{
public:
	void Init();
};
extern cHack Hack;

typedef void(*CL_Move_t)();

extern CL_Move_t CL_Move_s;

class PlayerInfoLocal
{
public:

	Vector vPostForward;

	CBasePlayerWeapon weapon;
};