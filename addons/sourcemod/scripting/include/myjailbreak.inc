/*
 * MyJailbreak - Include File.
 * by: shanapu
 * https://github.com/shanapu/MyJailbreak/
 *
 * This file is part of the MyJailbreak SourceMod Plugin.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/******************************************************************************
                   STARTUP
******************************************************************************/


#if defined _myjailbreak_included_
	#endinput
#endif
#define _myjailbreak_included_


public SharedPlugin:__pl_myjailbreak =
{
	name = "myjailbreak",
	file = "myjailbreak.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0,
	#endif
};


//Defines
#define PLUGIN_VERSION "Beta 7.0"
#define URL_LINK "https://github.com/shanapu/MyJailbreak"


//Includes
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <cstrike>
#include <smlib>
#include <emitsoundany>
#include <colors>
#include <autoexecconfig>
#include <smartjaildoors>
#include <hosties>
#include <lastrequest>
#include <warden>
#include <scp>


/******************************************************************************
                   NATIVES
******************************************************************************/


/*********************************************************
 * Set the name of the planned/running Event Day
 *
 * @param name			The name of the Event Day
 * @NoReturn
 *********************************************************/
native void SetEventDayName(const char[] name);


/*********************************************************
 * Get the name of the planned/running Event Day
 *
 *
 * @Return				The name of the Event Day
 *********************************************************/
native void GetEventDayName(char[] name);


/*********************************************************
 * Set Boolean Event Day is running
 *
 * @param status		Boolean is running (true = running)
 * @NoReturn
 *********************************************************/
native void SetEventDayRunning(bool status);


/*********************************************************
 * Check bool Event Day is running
 *
 *
 * @Return			true on running, false if not
 *********************************************************/
native bool IsEventDayRunning();


/*********************************************************
 * Set Boolean Event Day is planned
 *
 * @param status		Boolean is planned (true = running)
 * @NoReturn
 *********************************************************/
native void SetEventDayPlanned(bool status);


/*********************************************************
 * Check bool Event Day is planned
 *
 *
 * @Return			true on planned, false if not
 *********************************************************/
native bool IsEventDayPlanned();


/*********************************************************
 * Set Boolean Last Guard Rule is active
 *
 * @param status		Boolean is active (true = active)
 * @NoReturn
 *********************************************************/
native void SetLastGuardRule(bool status);


/*********************************************************
 * Check bool Last Guard Rule is active
 *
 *
 * @Return			true on active, false if not
 *********************************************************/
native bool IsLastGuardRule();


/*********************************************************
 * Check bool is logging active
 *
 *
 * @Return			true on active, false if not
 *********************************************************/
native bool ActiveLogging();


/*********************************************************
 * Set map fog
 *
 *
 * @noreturn
 *********************************************************/
native void FogOn();


/*********************************************************
 * Remove a map fog
 *
 *
 * @noreturn
 *********************************************************/
native void FogOff();


/*********************************************************
 * Set client Beacon
 *
 *
 * @noreturn
 *********************************************************/
native void BeaconOn(int client, float interval);


/*********************************************************
 * Remove client Beacon (auto remove on mapend/playerdeath/roundend)
 *
 *
 * @noreturn
 *********************************************************/
native void BeaconOff(int client);


/******************************************************************************
                   STOCKS
******************************************************************************/


//Check for valid clients with bool for bots & dead player 
stock bool IsValidClient(int client, bool bAllowBots = false, bool bAllowDead = true)
{
	if(!(1 <= client <= MaxClients) || !IsClientInGame(client) || (IsFakeClient(client) && !bAllowBots) || IsClientSourceTV(client) || IsClientReplay(client) || (!bAllowDead && !IsPlayerAlive(client)))
	{
		return false;
	}
	return true;
}


//Easy clean client loops
#define LoopClients(%1) for(int %1 = 1; %1 <= MaxClients; %1++) if(IsClientInGame(%1))
#define LoopValidClients(%1,%2,%3) for(int %1 = 1; %1 <= MaxClients; %1++) if(IsValidClient(%1, %2, %3))


//Easy precache & prepare download for decals
stock void PrecacheDecalAnyDownload(char[] sDecal)
{
	char sBuffer[256];
	Format(sBuffer, sizeof(sBuffer), "%s.vmt", sDecal);
	PrecacheDecal(sBuffer, true);
	Format(sBuffer, sizeof(sBuffer), "materials/%s.vmt", sDecal);
	AddFileToDownloadsTable(sBuffer);
	
	Format(sBuffer, sizeof(sBuffer), "%s.vtf", sDecal);
	PrecacheDecal(sBuffer, true);
	Format(sBuffer, sizeof(sBuffer), "materials/%s.vtf", sDecal);
	AddFileToDownloadsTable(sBuffer);
}


//Easy precache & prepare download for models (icons)
stock void PrecacheModelAnyDownload(char[] sModel)
{
	char sBuffer[256];
	Format(sBuffer, sizeof(sBuffer), "materials/%s.vmt", sModel);
	AddFileToDownloadsTable(sBuffer);
	PrecacheModel(sBuffer, true);
	Format(sBuffer, sizeof(sBuffer), "materials/%s.vtf", sModel);
	AddFileToDownloadsTable(sBuffer);
	PrecacheModel(sBuffer, true);
}


//Easy precache & prepare download for sounds
stock void PrecacheSoundAnyDownload(char[] sSound)
{
	char sBuffer[256];
	PrecacheSoundAny(sSound);
	Format(sBuffer, sizeof(sBuffer), "sound/%s", sSound);
	AddFileToDownloadsTable(sBuffer);
}


//Strip All Weapons & the knife slot twice for taser
stock void StripAllPlayerWeapons(int client)
{
	int weapon;
	for (int i = 0; i <= 4; i++)
	{
		if((weapon = GetPlayerWeaponSlot(client, i)) != -1)
		{
			SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR); 
			AcceptEntityInput(weapon, "Kill"); 
		}
	}
	if((weapon = GetPlayerWeaponSlot(client, CS_SLOT_KNIFE)) != -1)   //strip knife slot 2 times for taser
	{
		SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR); 
		AcceptEntityInput(weapon, "Kill"); 
	}
}


//Easy show overlay to a client with lifetime | 0.0 = no auto remove
stock void ShowOverlay(int client, char [] path, float lifetime) 
{
	if(IsValidClient(client, false, true))
	{
		int iFlag = GetCommandFlags( "r_screenoverlay" ) & ( ~FCVAR_CHEAT ); 
		SetCommandFlags( "r_screenoverlay", iFlag ); 
		ClientCommand( client, "r_screenoverlay \"%s.vtf\"", path);
		if (lifetime != 0.0) CreateTimer(lifetime, DeleteOverlay, client);
	}
}


//Easy show overlay to all clients with lifetime | 0.0 = no auto remove
stock void ShowOverlayAll(char [] path, float deletetime) 
{
	LoopValidClients(client, false, true)
	{
		int iFlag = GetCommandFlags( "r_screenoverlay" ) & ( ~FCVAR_CHEAT ); 
		SetCommandFlags( "r_screenoverlay", iFlag ); 
		ClientCommand( client, "r_screenoverlay \"%s.vtf\"", path);
		if (deletetime != 0.0) CreateTimer( deletetime, DeleteOverlay, client );
	}
}


//Remove overlay from a client
stock Action DeleteOverlay( Handle timer, any client ) 
{
	if(IsValidClient(client, false, true))
	{
		int iFlag = GetCommandFlags( "r_screenoverlay" ) & ( ~FCVAR_CHEAT ); 
		SetCommandFlags( "r_screenoverlay", iFlag ); 
		ClientCommand( client, "r_screenoverlay \"\"" );
	}
}


//Easy silent change of ConVars - Boolean
stock void SetCvar(char cvarName[64],int value)
{
	Handle IntCvar = FindConVar(cvarName);
	if(IntCvar == null) return;
	
	int flags = GetConVarFlags(IntCvar);
	flags &= ~FCVAR_NOTIFY;
	SetConVarFlags(IntCvar, flags);
	
	SetConVarInt(IntCvar, value);
	
	flags |= FCVAR_NOTIFY;
	SetConVarFlags(IntCvar, flags);
}


//Easy silent change of ConVars - Floats
stock void SetCvarFloat(char cvarName[64], float value)
{
	Handle FloatCvar = FindConVar(cvarName);
	if(FloatCvar == INVALID_HANDLE) return;
	
	new flags = GetConVarFlags(FloatCvar);
	flags &= ~FCVAR_NOTIFY;
	SetConVarFlags(FloatCvar, flags);
	
	SetConVarFloat(FloatCvar, value);
	
	flags |= FCVAR_NOTIFY;
	SetConVarFlags(FloatCvar, flags);
}


//Easy silent change of ConVars - Strings
stock void SetCvarString(char cvarName[64], char[] value)
{
	Handle cvar = FindConVar(cvarName);
	SetConVarString(cvar, value, true);
}


//Define filename & path for logging 
stock void SetLogFile(char LogFile[PLATFORM_MAX_PATH] , char [] FileName)
{
	char sDate[12];
	FormatTime(sDate, sizeof(sDate), "%y-%m-%d");
	Format(LogFile, sizeof(LogFile), "logs/MyJailbreak/%s-%s.log", FileName, sDate);
	
	BuildPath(Path_SM, LogFile, sizeof(LogFile), LogFile);
}


//Set the players Ammo for active weapon
stock void SetPlayerWeaponAmmo(int client, int weaponEnt, int clip = -1, int ammo = -1)
{
	if (weaponEnt == INVALID_ENT_REFERENCE)
		return;
	
	if (clip != -1)
		SetEntProp(weaponEnt, Prop_Send, "m_iClip1", clip);
	
	if (ammo != -1)
	{
		int iOffset = FindDataMapInfo(client, "m_iAmmo") + (GetEntProp(weaponEnt, Prop_Data, "m_iPrimaryAmmoType") * 4);
		SetEntData(client, iOffset, ammo, 4, true);
		
		if (GetEngineVersion() == Engine_CSGO) 
		{
			SetEntProp(weaponEnt, Prop_Send, "m_iPrimaryReserveAmmoCount", ammo);
		}
	}
}


//Remove a players render color
stock Action Timer_RemoveColor( Handle timer, any client ) 
{
	if(IsValidClient(client, true, true))
	{
		SetEntityRenderColor(client, 255, 255, 255, 255);
	}
}


//Get random alive & non-rebeling player of a team
stock int GetRandomPlayer(int team) 
{
	int[] clients = new int[MaxClients];
	int clientCount;
	LoopClients(i)
	{
		if ((GetClientTeam(i) == team) && IsPlayerAlive(i) && !IsClientRebel(i))
		{
			clients[clientCount++] = i;
		}
	}
	return (clientCount == 0) ? -1 : clients[GetRandomInt(0, clientCount-1)];
}


//Get alive & non-rebeling player count of a team
stock int GetAlivePlayersCount(int team)
{
	int iCount, i; iCount = 0;
	
	for( i = 1; i <= MaxClients; i++ )
		if(IsValidClient(i,true,false) && !IsClientRebel(i) && GetClientTeam( i ) == team )
		iCount++;
	
	return iCount;
}


//Get alive player count of a team
stock int GetAliveTeamCount(int team)
{
	int number = 0;
	LoopClients(i) if(IsPlayerAlive(i) && GetClientTeam(i) == team) number++;
	return number;
}


//Get a alive random player of all teams
stock int GetRandomAlivePlayer() 
{
	int[] clients = new int[MaxClients];
	int clientCount;
	LoopClients(i)
	{
		if (IsPlayerAlive(i))
		{
			clients[clientCount++] = i;
		}
	}
	return (clientCount == 0) ? -1 : clients[GetRandomInt(0, clientCount-1)];
}


//Get a player for a certain admin flag
stock bool CheckVipFlag(int client, const char[] flagsNeed)
{
	if((GetUserFlagBits(client) & ReadFlagString(flagsNeed) == ReadFlagString(flagsNeed)) || (GetUserFlagBits(client) & ADMFLAG_ROOT))
	{
		return true;
	}
	return false;
}


//Menu Handler for Panels
stock int Handler_NullCancel(Handle menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_Select) 
	{
		switch (param2) 
		{
			default: //cancel
			{
				return;
			}
		}
	}
	return;
}