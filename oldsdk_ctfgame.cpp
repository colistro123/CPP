#include "cbase.h"
#include "sdk_gamerules.h"
#include "sdk_ctfgame.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "weapon_sdkbase.h"

#include "decals.h"
#include "shake.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "basecombatweapon_shared.h"

#ifdef CLIENT_DLL
	#include "precache_register.h"
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "voice_gamemgr.h"
	#include "sdk_player.h"
	#include "sdk_team.h"
	#include "sdk_playerclass_info_parse.h"
	#include "player_resource.h"
	#include "mapentities.h"
	#include "sdk_basegrenade_projectile.h"
	#include "util.h"
	#include "world.h"
	#include "baseentity.h"
#endif

//Gamerules

REGISTER_GAMERULES_CLASS( CInheritedGameRules );

BEGIN_NETWORK_TABLE_NOBASE( CInheritedGameRules, DT_InheritedGameRules )

	#ifdef CLIENT_DLL
		RecvPropInt( RECVINFO( mi6_Flags )),
		RecvPropInt( RECVINFO( phoenix_Flags )),
		//RecvPropInt( RECVINFO( mi6_Flag_State )),
		RecvPropInt( RECVINFO( Flag_State )),
		RecvPropInt( RECVINFO( gameType )),
		//RecvPropInt( RECVINFO( gameInit )),
		RecvPropInt( RECVINFO( carryingFlag )),
	#else
		SendPropInt( SENDINFO( mi6_Flags ), 24, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( phoenix_Flags ), 24, SPROP_UNSIGNED ),
		//SendPropInt( SENDINFO( mi6_Flag_State ), 24, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( Flag_State ), 24, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( gameType ), 24, SPROP_UNSIGNED ),
		//SendPropInt( SENDINFO( gameInit ), 24, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( carryingFlag ), 24, SPROP_UNSIGNED ),
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( inherited_gamerules, CInheritedGameRulesProxy );

IMPLEMENT_NETWORKCLASS_ALIASED( InheritedGameRulesProxy, DT_InheritedGameRulesProxy )
/* Initialize all the CInheritedGameRules Vars */
int CInheritedGameRules::gameInit = 0;
/* Initialize all the MI6 Statics */
int MI6FlagObj::m_nThreshold = 0;
int MI6FlagObj::m_nCounter = 0;
int MI6FlagObj::lastFlagOwner = 0;
int MI6FlagObj::m_TeamName = 0;
Vector MI6FlagObj::m_flagResetPos;
QAngle MI6FlagObj::m_flagResetAng;
int MI6FlagObj::iAttachment = 0;
/* Initialize all the Phoenix Statics */
int PhoenixFlagObj::m_nThreshold = 0;
int PhoenixFlagObj::m_nCounter = 0;
int PhoenixFlagObj::lastFlagOwner = 0;
int PhoenixFlagObj::m_TeamName = 0;
Vector PhoenixFlagObj::m_flagResetPos;
QAngle PhoenixFlagObj::m_flagResetAng;
int PhoenixFlagObj::iAttachment = 0;
/* End of var initializations */

#ifdef CLIENT_DLL
	void RecvProxy_InheritedGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID ) {
		CInheritedGameRules *pRules = InheritedGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CInheritedGameRulesProxy, DT_InheritedGameRulesProxy )
		RecvPropDataTable( "inherited_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_InheritedGameRules ), RecvProxy_InheritedGameRules )
	END_RECV_TABLE()
#else
	void* SendProxy_InheritedGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID ) {
		CInheritedGameRules *pRules = InheritedGameRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CInheritedGameRulesProxy, DT_InheritedGameRulesProxy )
		SendPropDataTable( "inherited_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_InheritedGameRules ), SendProxy_InheritedGameRules )
	END_SEND_TABLE()
#endif

CInheritedGameRules::CInheritedGameRules() {
	//Constructor...
	#ifdef GAME_DLL
	tryToInitializeCTF();
	#endif
}
CInheritedGameRules::~CInheritedGameRules( void ) {
#ifdef GAME_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	//g_Teams.Purge();
#endif
}
#ifdef GAME_DLL
void CInheritedGameRules::tryToInitializeCTF() {
	// Detect if the gamemode is CTF
	if(gameInit != 1) {
		CWorld *world = GetWorldEntity();
		if ( world && world->GetDefaultCTF() == 1 ) {
			InitializeCTFGame();
			Warning("tryToInitializeCTF() was called \n");
			Warning("Initializing CTF gamemode..\n");
			Warning("CTF Seems to be loaded and working....\n");
		}
	}
}
void CInheritedGameRules::InitializeCTFGame( void ) {
	//code here
	#ifdef GAME_DLL
	mi6_Flags = 0;
	phoenix_Flags = 0;
	Flag_State = FLAG_STATE_BASE;
	gType = GAMEMODE_CTF;
	gameInit = 1;
	Warning("InitializeCTFGame() called: Gamemode: CTF, gametype %d\n", this->gType);
	#endif
}
#endif
void CInheritedGameRules::Think( void ) {
#ifdef GAME_DLL
	//Warning("CTF: I'm Thinking!\n");
	BaseClass::Think();

#endif
}
void CInheritedGameRules::tryDropFlag( void ) {
	MI6FlagObj *flagObject = new MI6FlagObj();
	if(!flagObject) {
		return;
	}
	for(int i=0; i<gpGlobals->maxClients; i++) {
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if(pPlayer /* && pPlayer->m_iTeam == 1 */) {
			if(pPlayer->IsAlive()) { //If the player is alive
				if(flagObject->hasFlag(pPlayer)) { //If the player has the flag
					if(flagObject->GetFlagState() == FLAG_STATE_STOLEN) { //If it's stolen
						Warning("Flag dropped\n");
						flagObject->resetFlag();
					}
				}
			}
		}
	}
}
//Flags
/* ====================================== MI6 Flag ====================================== */
LINK_ENTITY_TO_CLASS( ctfflag_mi6, MI6FlagObj ); //Also load the flag bases here
//LINK_ENTITY_TO_CLASS( item_ctfbase, MI6FlagObj );

BEGIN_DATADESC( MI6FlagObj )

	DEFINE_FIELD( m_nCounter, FIELD_INTEGER ),
	DEFINE_FIELD( m_TeamName, FIELD_INTEGER ),
	// As above, and also links our member variable to a Hammer keyvalue
	DEFINE_KEYFIELD( m_nThreshold, FIELD_INTEGER, "threshold" ),

END_DATADESC()

void MI6FlagObj::SetFlagState( int iState ) { //Flag animation states
	MDLCACHE_CRITICAL_SECTION(); //To prevent the flags from being deleted in case the system runs in low memory
	CInheritedGameRules *flagInfo = new CInheritedGameRules();
	if(!flagInfo) {
		return;
	}
	switch ( iState ) {
	case FLAG_STATE_BASE:
		flagInfo->Flag_State = FLAG_STATE_BASE;
		ResetSequence( LookupSequence("flag_positioned") );
		//ResetSequence( LookupSequence("wave_idle") );
	case FLAG_STATE_DROPPED:
		flagInfo->Flag_State = FLAG_STATE_DROPPED;
		ResetSequence( LookupSequence("on_ground") );
		break;
	case FLAG_STATE_STOLEN:
		flagInfo->Flag_State = FLAG_STATE_STOLEN;
		ResetSequence( LookupSequence("carried") );
		break;
	default:
		AssertOnce( false );	// invalid stats
		break;
	}
}
int MI6FlagObj::GetFlagState() { //Flag animation states
	MDLCACHE_CRITICAL_SECTION(); //To prevent the flags from being deleted in case the system runs in low memory
	CInheritedGameRules *flagInfo = new CInheritedGameRules();
	if(flagInfo) {
		return flagInfo->Flag_State;
	}
	return 0;
}
void MI6FlagObj::setFlagOwner( int ownerid ) {
	lastFlagOwner = ownerid;
}
void MI6FlagObj::Spawn( void ) {
	// Precache the model and sounds.  Set the flag model.
	Warning("Flags spawned Team: %d\n", m_TeamName);
	Precache();
	SetModel( FLAG_MI6_MDL );
	// Set the flag solid and the size for touching.
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetSize( vec3_origin, vec3_origin );
	SetPlaybackRate( 1.0f );
	// Tell the client to animate this model
	UseClientSideAnimation();
	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds( true, 24 );
	SetTouch(&MI6FlagObj::Touch);
	// Base class spawn.
	BaseClass::Spawn();

#ifdef GAME_DLL
	// Save the starting position, so we can reset the flag later if need be.
	m_flagResetPos = GetAbsOrigin();
	m_flagResetAng = GetAbsAngles();
	SetFlagState( FLAG_STATE_BASE );
#endif
}
bool MI6FlagObj::hasFlag(CBasePlayer *pPlayer) {
	if(pPlayer == NULL) {
		Warning("MI6FlagObj::hasFlag: pPlayer: %d return: 0\n",pPlayer->GetUserID());
		return 0;
	}
	if(flagOwner() == pPlayer->GetUserID()) { //If the last flagowner equals the userid, we know that he is the one who has the flag
		Warning("MI6FlagObj::hasFlag: pPlayer: %d return: 1\n",pPlayer->GetUserID());
		return 1;
	}
	return 0;
}
#ifdef GAME_DLL
bool MI6FlagObj::KeyValue( const char *szKeyName, const char *szValue ) {
	Warning("szKeyName: %s, szValue: %s\n", szKeyName, szValue);
	if (FStrEq(szKeyName, "goal_no")) {
		m_TeamName = atoi(szValue);
		if(m_TeamName == 1) {
			m_TeamName = SDK_TEAM_MI6;
		} else {
			m_TeamName = SDK_TEAM_PHOENIX;
		}
		Warning("Team: %d\n", m_TeamName);
		//TEAM_NO 1 == MI6 in FGD (It's 2 in SDK because it's incremented due to the LAST_SHARED_TEAM+1), 2 == Phoenix in FGD, (It's 3 in SDK because it's incremented due to the LAST_SHARED_TEAM+1)
	}
	else if (FStrEq(szKeyName, "lip") ) {
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}
#endif
void MI6FlagObj::Touch(CBaseEntity *pOther) {
    // Determine if the object that touches it, is a player
    // and check if the player is alive.
	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
    if(!pOther)
         return;
    if(!pOther->IsPlayer())
         return;
    if(!pOther->IsAlive())
         return;
	if(pPlayer->GetTeamNumber() != m_TeamName) { //If we touch the enemy's flag (not ours)
		const char* playername;
		playername = pPlayer->GetPlayerName();
		iAttachment = pPlayer->LookupAttachment( "ctfflag_mi6" );
		#ifdef GAME_DLL
		// Print to the HUD who has taken the flag
		UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s has taken the %s Flag!\n", playername, MI6_TEAM_NAME));
		//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s has the %s Flag!\n", MI6_TEAM_NAME, m_TeamName));
		#endif
		// Set the client attachment using an event
		if( iAttachment != -1 ) {
			SetParent( pPlayer, iAttachment );
			SetLocalOrigin( vec3_origin );
			SetLocalAngles( vec3_angle );
			this->setFlagOwner(pPlayer->GetUserID());
		}
		Warning("MI6FlagObj::Touch:Userid: %d, Flag Owner: %d\n", pPlayer->GetUserID(), flagOwner());
		SetFlagState( FLAG_STATE_STOLEN );
		EmitSound( MI6_SOUND_FLAG_TAKEN );
		//Set the flag here
		// Now don't let it accept any more touch's
		SetTouch(NULL);
	} else { //If we do, call this
		checkCapture(pOther, m_TeamName); //call this function
		// Now don't let it accept any more touch's
		SetTouch(NULL);
	}
}
void MI6FlagObj::checkCapture( CBaseEntity *pOther, int teamNameID) {
	if(pOther == NULL) {
		return;
	}
	PhoenixFlagObj *flagInfoPhoenix = new PhoenixFlagObj(); //Because we need to get the status from the opposite flag
	if(flagInfoPhoenix == NULL) {
		return;
	}
	const char* playername;
	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	playername = pPlayer->GetPlayerName();
	Warning("MI6FlagObj::checkCapture() was called hasFlag: %d, GetTeamNumber: %d, TeamNameID: %d\n", flagInfoPhoenix->hasFlag(pPlayer),pPlayer->GetTeamNumber(),teamNameID);
	if(pPlayer && pPlayer->GetTeamNumber() == teamNameID) { //If we are at our flag
		if(flagInfoPhoenix->hasFlag(pPlayer)) { //If the player has the Phoenix flag
			#ifdef GAME_DLL
					UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s has taken the %s Flag!\n", playername, MI6_TEAM_NAME));
			#endif
			//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "TeamNumber: %d FlagTeamID: %d Flag!\n", pPlayer->GetTeamNumber(), teamNameID));
			flagInfoPhoenix->resetFlag(); //Reset the enemy flag
			addScore(1); //Add 1 to the current "captured flags" score.
			EmitSound( MI6_SOUND_FLAG_CAPTURED );
		}
	}
}
void MI6FlagObj::addScore( int amount ) {
	if(amount != 0) {
		int actualScore = this->m_nCounter;
		this->m_nCounter = actualScore + amount;
	}
}
void MI6FlagObj::Precache( void ) {
	Warning("Precache called \n");
	PrecacheModel( FLAG_MI6_MDL );
	PrecacheScriptSound( MI6_SOUND_FLAG_TAKEN );
	PrecacheScriptSound( MI6_SOUND_FLAG_CAPTURED );
	BaseClass::Precache();
}
void MI6FlagObj::resetFlag( void ) {
//#ifdef GAME_DLL
	// Set the flag position.
	SetParent( NULL );
	Warning("Flag position reset\n");
	SetAbsOrigin( m_flagResetPos );
	SetAbsAngles( m_flagResetAng );
	#ifdef GAME_DLL
	m_hReturnIcon = CBaseEntity::Create( "ctfflag_mi6", m_flagResetPos, m_flagResetAng, this );
	UTIL_Remove( this );
	#endif
	//Reset its owner
	this->setFlagOwner(-1);
	// No longer dropped, if it was.
	SetFlagState( FLAG_STATE_BASE );
	SetMoveType( MOVETYPE_NONE );
//#endif
}
/* ====================================== Phoenix Flag ====================================== */
LINK_ENTITY_TO_CLASS( ctfflag_phoenix, PhoenixFlagObj ); //Also load the flag bases here
//LINK_ENTITY_TO_CLASS( item_ctfbase, MI6FlagObj );

BEGIN_DATADESC( PhoenixFlagObj )

	DEFINE_FIELD( m_nCounter, FIELD_INTEGER ),
	DEFINE_FIELD( m_TeamName, FIELD_INTEGER ),
	// As above, and also links our member variable to a Hammer keyvalue
	DEFINE_KEYFIELD( m_nThreshold, FIELD_INTEGER, "threshold" ),

END_DATADESC()

void PhoenixFlagObj::SetFlagState( int iState ) { //Flag animation states
	MDLCACHE_CRITICAL_SECTION(); //To prevent the flags from being deleted in case the system runs in low memory
	CInheritedGameRules *flagInfo = new CInheritedGameRules();
	if(!flagInfo) {
		return;
	}
	switch ( iState ) {
	case FLAG_STATE_BASE:
		flagInfo->Flag_State = FLAG_STATE_BASE;
		ResetSequence( LookupSequence("flag_positioned") );
		//ResetSequence( LookupSequence("wave_idle") );
	case FLAG_STATE_DROPPED:
		flagInfo->Flag_State = FLAG_STATE_DROPPED;
		ResetSequence( LookupSequence("on_ground") );
		break;
	case FLAG_STATE_STOLEN:
		flagInfo->Flag_State = FLAG_STATE_STOLEN;
		ResetSequence( LookupSequence("carried") );
		break;
	default:
		AssertOnce( false );	// invalid stats
		break;
	}
}
int PhoenixFlagObj::GetFlagState() { //Flag animation states
	MDLCACHE_CRITICAL_SECTION(); //To prevent the flags from being deleted in case the system runs in low memory
	CInheritedGameRules *flagInfo = new CInheritedGameRules();
	if(flagInfo) {
		return flagInfo->Flag_State;
	}
	return 0;
}
void PhoenixFlagObj::setFlagOwner( int ownerid ) {
	lastFlagOwner = ownerid;
}
void PhoenixFlagObj::Spawn( void ) {
	// Precache the model and sounds.  Set the flag model.
	Warning("Flags spawned Team: %d\n", m_TeamName);
	Precache();
	SetModel( FLAG_MDL );
	// Set the flag solid and the size for touching.
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetSize( vec3_origin, vec3_origin );
	SetPlaybackRate( 1.0f );
	// Tell the client to animate this model
	UseClientSideAnimation();
	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds( true, 24 );
	SetTouch(&PhoenixFlagObj::Touch);
	// Base class spawn.
	BaseClass::Spawn();

#ifdef GAME_DLL
	// Save the starting position, so we can reset the flag later if need be.
	m_flagResetPos = GetAbsOrigin();
	m_flagResetAng = GetAbsAngles();
	SetFlagState( FLAG_STATE_BASE );
#endif
}
bool PhoenixFlagObj::hasFlag(CBasePlayer *pPlayer) {
	if(pPlayer == NULL) {
		Warning("PhoenixFlagObj::hasFlag: pPlayer: %d return: 0\n",pPlayer->GetUserID());
		return 0;
	}
	if(flagOwner() == pPlayer->GetUserID()) { //If the last flagowner equals the userid, we know that he is the one who has the flag
		Warning("PhoenixFlagObj::hasFlag: pPlayer: %d return: 1\n",pPlayer->GetUserID());
		return 1;
	}
	return 0;
}
#ifdef GAME_DLL
bool PhoenixFlagObj::KeyValue( const char *szKeyName, const char *szValue ) {
	Warning("szKeyName: %s, szValue: %s\n", szKeyName, szValue);
	if (FStrEq(szKeyName, "goal_no")) {
		m_TeamName = atoi(szValue);
		if(m_TeamName == 1) {
			m_TeamName = SDK_TEAM_MI6;
		} else {
			m_TeamName = SDK_TEAM_PHOENIX;
		}
		Warning("Team: %d\n", m_TeamName);
		//TEAM_NO 1 == MI6 in FGD (It's 2 in SDK because it's incremented due to the LAST_SHARED_TEAM+1), 2 == Phoenix in FGD, (It's 3 in SDK because it's incremented due to the LAST_SHARED_TEAM+1)
	}
	else if (FStrEq(szKeyName, "lip") ) {
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}
#endif
void PhoenixFlagObj::Touch(CBaseEntity *pOther) {
    // Determine if the object that touches it, is a player
    // and check if the player is alive.
	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
    if(!pOther)
         return;
    if(!pOther->IsPlayer())
         return;
    if(!pOther->IsAlive())
         return;
	if(pPlayer->GetTeamNumber() != m_TeamName) { //If we touch the enemy's flag (not ours)
		const char* playername;
		playername = pPlayer->GetPlayerName();
		iAttachment = pPlayer->LookupAttachment( "ctfflag_phoenix" );
		#ifdef GAME_DLL
		// Print to the HUD who has taken the flag
		UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s has the %s Flag!\n", playername, PHOENIX_TEAM_NAME));
		#endif
		// Set the client attachment using an event
		if( iAttachment != -1 ) {
			SetParent( pPlayer, iAttachment );
			SetLocalOrigin( vec3_origin );
			SetLocalAngles( vec3_angle );
			this->setFlagOwner(pPlayer->GetUserID());
		}
		Warning("PhoenixFlagObj::Userid: %d, Flag Owner: %d\n", pPlayer->GetUserID(), flagOwner());
		SetFlagState( FLAG_STATE_STOLEN );
		//set the flag here
		// Now don't let it accept any more touch's
		EmitSound( PHOENIX_SOUND_FLAG_TAKEN );
		SetTouch(NULL);
	} else { //If we do, call this
		checkCapture(pOther, m_TeamName); //call this function
		// Now don't let it accept any more touch's
		SetTouch(NULL);
	}
}
void PhoenixFlagObj::checkCapture( CBaseEntity *pOther, int teamNameID) {
	if(pOther == NULL) {
		return;
	}
	MI6FlagObj *flagInfoMI6 = new MI6FlagObj(); //Because we need to get the status from the opposite flag
	if(flagInfoMI6 == NULL) {
		return;
	}
	const char* playername;
	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	playername = pPlayer->GetPlayerName();
	Warning("PhoenixFlagObj::checkCapture() was called hasFlag: %d, GetTeamNumber: %d, TeamNameID: %d\n", flagInfoMI6->hasFlag(pPlayer),pPlayer->GetTeamNumber(),teamNameID);
	if(pPlayer && pPlayer->GetTeamNumber() == teamNameID) { //If we are at our flag
		if(flagInfoMI6->hasFlag(pPlayer)) { //If the player has the Phoenix flag
			#ifdef GAME_DLL
			UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s has captured the %s Flag!\n", playername, MI6_TEAM_NAME));
			#endif
			//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "TeamNumber: %d FlagTeamID: %d Flag!\n", pPlayer->GetTeamNumber(), teamNameID));
			flagInfoMI6->resetFlag(); //Reset the enemy flag
			addScore(1); //Add 1 to the current "captured flags" score.
			EmitSound( PHOENIX_SOUND_FLAG_CAPTURED );
		}
	}
}
void PhoenixFlagObj::addScore( int amount ) {
	if(amount != 0) {
		int actualScore = this->m_nCounter;
		this->m_nCounter = actualScore + amount;
	}
}
void PhoenixFlagObj::Precache( void ) {
	Warning("Precache called \n");
	PrecacheModel( FLAG_MDL );
	PrecacheScriptSound( PHOENIX_SOUND_FLAG_TAKEN );
	PrecacheScriptSound( PHOENIX_SOUND_FLAG_CAPTURED );
	BaseClass::Precache();
}
void PhoenixFlagObj::resetFlag( void ) {
//#ifdef GAME_DLL
	// Set the flag position.
	
	Warning("Flag position reset\n");
	SetAbsOrigin( m_flagResetPos );
	SetAbsAngles( m_flagResetAng );
	#ifdef GAME_DLL
	//SetParent( NULL, iAttachment );
	SetOwnerEntity( NULL );
	m_hReturnIcon = CBaseEntity::Create( "ctfflag_phoenix", m_flagResetPos, m_flagResetAng, this );
	UTIL_Remove( this );
	#endif
	//Reset its owner
	this->setFlagOwner(-1);
	// No longer dropped, if it was.
	SetFlagState( FLAG_STATE_BASE );
	SetMoveType( MOVETYPE_NONE );
//#endif
}
