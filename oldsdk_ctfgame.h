/*
	PURPOSE: CTF Gamemode.
*/

#ifndef CTF_GAMERULES_H
#define CTF_GAMERULES_H
#pragma once

#if !defined( CLIENT_DLL )
#include "entitylist.h"
#endif

#include "studio.h"
#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "weapon_sdkbase.h"
#include "sdk_gamerules.h"
#include "eiface.h"
#include "KeyValues.h"
#include "imovehelper.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
	#include "c_baseentity.h"
#else
	#include "player.h"
	#include "utlqueue.h"
	#include "playerclass_info_parse.h"
	#include "baseentity.h"
	#include "items.h"
#endif

#ifdef CLIENT_DLL
	#define CCTFGameRules C_CTFGameRules
	#define CCTFGameRulesProxy C_CTFGameRulesProxy
#endif
//----------------------------------------------------------------------------
//					Gamerules Defines
//----------------------------------------------------------------------------
//Game Modes
#define GAMEMODE_CT 0
#define GAMEMODE_CTF 1

//Team Names (Standalone)
#define MI6_TEAM_NAME "MI6"
#define PHOENIX_TEAM_NAME "Phoenix"

//Flag States:
#define FLAG_STATE_BASE 0
#define FLAG_STATE_STOLEN 1
#define FLAG_STATE_DROPPED 2

//Think defines
#define CTF_FLAG_THINK_TIME			1.0f //1 second

//Flag Models:
#define FLAG_MDL	"models/ctf_flag.mdl"
#define FLAG_MI6_MDL	"models/ctf_flag_mi6.mdl"

//Sounds
#define MI6_SOUND_FLAG_TAKEN "ctf/mi6_flagtaken.wav"
#define MI6_SOUND_FLAG_CAPTURED "ctf/mi6_ctf.wav"
#define PHOENIX_SOUND_FLAG_TAKEN "ctf/phoenix_flagtaken.wav"
#define PHOENIX_SOUND_FLAG_CAPTURED "ctf/phoenix_ctf.wav"
//----------------------------------------------------------------------------

class CCTFGameRulesProxy : public CSDKGameRulesProxy
{
public:
	DECLARE_CLASS( CCTFGameRulesProxy, CSDKGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CCTFGameRules : public CSDKGameRules
{
public:
	DECLARE_CLASS( CCTFGameRules, CSDKGameRules );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif

	// Constructor
	CCTFGameRules();
	// Destructor
	virtual ~CCTFGameRules();

	virtual void Think( void );
	virtual const char *GetGameDescription( void ){ return "Nightfire Source: CTF"; } //Gamemode name
	void tryToInitializeCTF( void );
	void InitializeCTFGame( void );
	//void handleThink( void );
public:
	int gType;

	CNetworkVar( int, gameType );
	CNetworkVar( int, mi6_Flags );
	CNetworkVar( int, phoenix_Flags );
	CNetworkHandle( CBaseEntity, m_hPrevOwner ); //IGNACIO: theres two of these, where to put this?
	//Flag states: Stolen, captured, etc
	//CNetworkVar( int, Flag_State );
	//CNetworkVar( int, phoenix_Flag_State );
	//CNetworkVar( int, gameInit ); //Is our game initialized?
	CNetworkVar( int, carryingFlag );
	CHandle<CCTFGameRules>		m_hCTFGameRules;
private:
	static int gameInit;

};

inline CCTFGameRules* CTFGameRules()
{
	return static_cast<CCTFGameRules*>(g_pGameRules);
}

//Flag objects classes
class MI6FlagObj : public CBaseAnimating
{
private:
		CNetworkHandle( CBaseEntity, m_hPrevOwner );
public:
	DECLARE_CLASS( MI6FlagObj, CBaseAnimating );
	DECLARE_DATADESC();
	/*
	MI6FlagObj()
	{
		m_bActive = false;
	}
	*/
	void resetFlag( void );
	void MoveThink( void );
	void SetFlagState( int iState );
	// Input function
	void InputToggle( inputdata_t &inputData );
	
	void Spawn( void );
	void Precache( void );
	void Touch(CBaseEntity *);
	int GetFlagState();
	virtual bool hasFlag(CBasePlayer *pPlayer);
	virtual int flagOwner() { return lastFlagOwner; }
	virtual int getCTFAttachment() { return iAttachment; }
	void setFlagOwner(int ownerid);
	void checkCapture( CBaseEntity *pOther, int teamNameID);
	void addScore( int amount );
	void tryDropFlag( void );
	CBaseEntity		*GetPrevOwner( void ) { return m_hPrevOwner.Get(); }
#ifdef GAME_DLL
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
#endif
	CHandle<MI6FlagObj>		m_hMI6Flag;
private:
	EHANDLE m_hReturnIcon;
	bool	m_bActive;
	float	m_flNextChangeTime;
	static int	m_nThreshold;	// Count at which to fire our output
	static int	m_nCounter;	// Internal counter
	static Vector			m_flagResetPos;		// The position the flag should respawn (reset) at.
	static QAngle			m_flagResetAng;		// The angle the flag should respawn (reset) at.
	static int m_TeamName;				// Name or classname
	static int lastFlagOwner;
	static int iAttachment;
	static int Flag_State;
};

//Flag object class
class PhoenixFlagObj : public CBaseAnimating
{
private:
		CNetworkHandle( CBaseEntity, m_hPrevOwner );
public:
	DECLARE_CLASS( PhoenixFlagObj, CBaseAnimating );
	DECLARE_DATADESC();
	/*
	PhoenixFlagObj()
	{
		m_bActive = false;
	}
	*/
	void resetFlag( void );
	void MoveThink( void );
	void SetFlagState( int iState );
	// Input function
	void InputToggle( inputdata_t &inputData );
	
	void Spawn( void );
	void Precache( void );
	void Touch(CBaseEntity *);
	int GetFlagState();
	virtual bool hasFlag(CBasePlayer *pPlayer);
	virtual int flagOwner() { return lastFlagOwner; }
	void setFlagOwner(int ownerid);
	void checkCapture( CBaseEntity *pOther, int teamNameID);
	virtual int getCTFAttachment() { return iAttachment; }
	void addScore( int amount );
	void tryDropFlag( void );
	CBaseEntity		*GetPrevOwner( void ) { return m_hPrevOwner.Get(); }
#ifdef GAME_DLL
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
#endif
	CHandle<PhoenixFlagObj>		m_hPhoenixFlag;
private:
	EHANDLE m_hReturnIcon;
	bool	m_bActive;
	float	m_flNextChangeTime;
 
	static int	m_nThreshold;	// Count at which to fire our output
	static int	m_nCounter;	// Internal counter
	static Vector			m_flagResetPos;		// The position the flag should respawn (reset) at.
	static QAngle			m_flagResetAng;		// The angle the flag should respawn (reset) at.
	static int m_TeamName;				// Name or classname
	static int lastFlagOwner;
	static int iAttachment;
	static int Flag_State;
};
//Definitions

#endif //CTF_GAMERULES_H
