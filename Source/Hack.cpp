#include "Main.h"
#include "Hack.h"

playermove_t* pmove = NULL;
CL_Move_t CL_Move_s = NULL;

cHack Hack;

bool bStrafe;

void strafeON() { bStrafe = true; }
void strafeOFF() { bStrafe = false; }

bool strafe_silent = true;

void CL_Move();

PlayerInfoLocal g_Local;

void bSendpacket(bool status)
{
	static bool bsendpacket_status = true;
	static DWORD NULLTIME = NULL;

	if (status && !bsendpacket_status) {
		bsendpacket_status = true;

		*(DWORD*)(offset.dwSendPacketPointer) = offset.dwSendPacketBackup;
	}
	if (!status && bsendpacket_status) {
		bsendpacket_status = false;

		*(DWORD*)(offset.dwSendPacketPointer) = (DWORD)&NULLTIME;
	}
}

float YawForVec(float* fwd)
{
	if (fwd[1] == 0 && fwd[0] == 0)
		return 0;
	else
	{
		float yaw = (atan2(fwd[1], fwd[0]) * 180 / M_PI);
		if (yaw < 0)yaw += 360;
		return yaw;
	}
}

void CL_Move()
{
	bSendpacket(true);
	CL_Move_s();
}

void StrafeHack(struct usercmd_s* cmd)
{
	static bool packet = false;
	if (bStrafe && !(pmove->flags & FL_ONGROUND) && (pmove->movetype != MOVETYPE_FLY) && !(cmd->buttons & IN_ATTACK) && !(cmd->buttons & IN_ATTACK2 && g_Local.weapon.m_iWeaponID == WEAPON_KNIFE))
	{
		if (strafe_silent ? !packet : 1)
		{
			if (strafe_silent)bSendpacket(false);
			if (sqrt(POW(pmove->velocity[0]) + POW(pmove->velocity[1])) < 15)
				cmd->forwardmove = 400, cmd->sidemove = 0;

			float dir = 0;
			if (cmd->buttons & IN_MOVERIGHT)
				dir = 90;
			if (cmd->buttons & IN_BACK)
				dir = 180;
			if (cmd->buttons & IN_MOVELEFT)
				dir = -90;

			Vector ViewAngles;
			g_Engine.GetViewAngles(ViewAngles);
			ViewAngles.y += dir;
			float vspeed[3] = { pmove->velocity.x / pmove->velocity.Length(),pmove->velocity.y / pmove->velocity.Length(),0.0f };
			float va_speed = YawForVec(vspeed);
			float adif = va_speed - ViewAngles.y;
			while (adif < -180)adif += 360;
			while (adif > 180)adif -= 360;
			cmd->sidemove = (437.8928) * (adif > 0 ? 1 : -1);
			cmd->forwardmove = 0;
			bool onlysidemove = (abs(adif) >= atan(30.f / sqrt(POW(pmove->velocity[0]) + POW(pmove->velocity[1]))) / M_PI * 180);
			cmd->viewangles[1] -= (-adif);
			float fs = 0;
			if (!onlysidemove)
			{
				static float lv = 0;
				Vector fw = g_Local.vPostForward; fw[2] = 0; fw = fw.Normalize();
				float vel = POW(fw[0] * pmove->velocity[0]) + POW(fw[1] * pmove->velocity[1]);

				fs = lv;
				lv = sqrt(69.f * 100000 / vel);
				static float lastang = 0;
				float ca = abs(adif);
				lastang = ca;
			}

			float ang = atan(fs / cmd->sidemove) / M_PI * 180;
			cmd->viewangles.y += ang;

			float sdmw = cmd->sidemove;
			float fdmw = cmd->forwardmove;

			if (cmd->buttons & IN_MOVERIGHT)
				cmd->forwardmove = -sdmw, cmd->sidemove = fdmw;
			if (cmd->buttons & IN_BACK)
				cmd->forwardmove = -fdmw, cmd->sidemove = -sdmw;
			if (cmd->buttons & IN_MOVELEFT)
				cmd->forwardmove = sdmw, cmd->sidemove = -fdmw;
		}
		if (strafe_silent)packet = !packet;
	}
	else
		packet = false;
}

void CL_CreateMove(float frametime, struct usercmd_s* cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);

	if (bStrafe)
		StrafeHack(cmd);
}

void cHack::Init()
{
	g_pEngine->pfnAddCommand("+^strafe", strafeON);
	g_pEngine->pfnAddCommand("-^strafe", strafeOFF);

	g_pClient->CL_CreateMove = CL_CreateMove;

	while (!pmove)
		pmove = (playermove_t*)offset.FindPlayerMove();

	offset.GlobalTime();

	CL_Move_s = (CL_Move_t)DetourFunction((LPBYTE)offset.CL_Move(), (LPBYTE)&CL_Move);
}