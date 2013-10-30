//=================== Old Grapple Code I made for Nightfire Source ===================//
//
// Purpose: Grapple
//
//====================================================================================//

//TODOS: UNDONE, DONT START MOVING PLAYER UNTIL HOOK HITS THE WALL FIRST

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"
#include "NPCEvent.h"
#include "basecombatweapon_shared.h"
#include "gamemovement.h"
//#include "basecombatweapon.h"
//#include "basecombatcharacter.h"
//#include "AI_BaseNPC.h"
//#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
//#include "soundent.h"
//#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "in_buttons.h"
#include "sdk_fx_shared.h"
#include "igamemovement.h"
#include "vphysics/constraints.h"
#include "physics_saverestore.h"
#include "datamap.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "vcollide_parse.h"
#include "studio.h"
#include "bone_setup.h"
#include "collisionutils.h"
#include "beam_shared.h"

#define GRAPPLE_AIR_VELOCITY	450 //how fast player the player flies | OLD: 1000
#define GRAPPLE_WATER_VELOCITY	500
#define HOOK_SPRITE			"sprites/grapple_point.vmt" //Hook sprite
#define HOOK_MODEL			"models/whatever" //The hook model would go here but I don't know yet

#if defined( CLIENT_DLL )

	#define CWeaponGrapple C_WeaponGrapple
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

class CWeaponGrapple : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS( CWeaponGrapple, CWeaponSDKMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	CWeaponGrapple();

	virtual SDKWeaponID GetWeaponID( void ) const		{	return SDK_WEAPON_GRAPPLE; }
	virtual float	GetRange( void )					{	return	64.0f;	}	//Tony; let the crowbar swing further.
	virtual bool CanWeaponBeDropped() const				{	return false; }
	void	PrimaryAttack( void );
	virtual bool GrappleHitSomething( void );
	virtual bool IsPlayerGrappling( void );
	void	decideVecAimingHook( void );
	void	tryUngrappleOnEnd( void );
	void	Spawn( void );
	void	FireHook( void );
	void	ItemPreFrame( void );
	void	tempBlockWeapon( float seconds );
	void	setHookUpdateTime( float seconds );
	void	UpdatePlayerBeam( void );
	void	calculateTimeToSurface( void );
	virtual bool	hookIsThere( void );
	void	Precache( void );
	void	movePlayerIntoVector( void );
	//Global vectors
	Vector vecAimingRemember;
	Vector vecAbsEnd;
	Vector vecAbsStart;
	Vector FinalPos;
	Vector vecDir;
	//Floats
	float						m_secondsToSurface;
	//Integers
	int m_flGrappleNextUpdateTime;

private:

	//Functions
	void	MakeBeam( void );
	void	KillBeam( void );
	//CHandles
	CHandle<CBasePlayer>		m_hPlayer;
	CHandle<CBeam>			m_hBeam;
	//Network vars and other privates
	CWeaponGrapple( const CWeaponGrapple & );
	CNetworkVar( bool, pHook );
	CNetworkVar( bool, m_playerIsGrappled )
};

extern float IntervalDistance( float x, float x0, float x1 );

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrapple, DT_WeaponGrapple )

BEGIN_NETWORK_TABLE( CWeaponGrapple, DT_WeaponGrapple )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_playerIsGrappled ) ),
	RecvPropBool( RECVINFO( pHook ) ),
#else
	SendPropBool( SENDINFO( m_playerIsGrappled ) ),
	SendPropBool( SENDINFO( pHook ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrapple )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grapple, CWeaponGrapple );


PRECACHE_WEAPON_REGISTER( weapon_grapple );

void CWeaponGrapple::Precache()
{
	PrecacheModel( HOOK_SPRITE );
	BaseClass::Precache();
}

void CWeaponGrapple::PrimaryAttack( void )
{
	WeaponSound( EMPTY );
}

void CWeaponGrapple::ItemPreFrame( void ) {
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	UpdatePlayerBeam();
	if ((( pOwner->m_nButtons & IN_ATTACK ) == true )) { //Mouse button is pressed
		if(IsPlayerGrappling()) {
			decideVecAimingHook();
		} else {
			FireHook();
		}
	} else {
		m_playerIsGrappled = false;
	}
}

bool CWeaponGrapple::IsPlayerGrappling( void )
{
	//DevMsg("IsPlayerGrappling called \n");
	if(m_playerIsGrappled == true) {
		//DevMsg("The player is grappling \n");
		return 1;
	} else {
		return 0;
	}
}

bool CWeaponGrapple::GrappleHitSomething( void )
{
	//DevMsg("GrappleHitSomething called \n");
	
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
 
	//Create our trace_t class to hold the end result
	trace_t tr;
 
	//Take the Player's EyeAngles and turn it into a direction
	AngleVectors( pPlayer->EyeAngles(), &vecDir );
 
	//Get the Start/End
	vecAbsStart = pPlayer->EyePosition();
	vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);
 
	//Do the TraceLine, and write our results to our trace_t class, tr.
	UTIL_TraceLine( vecAbsStart, vecAbsEnd, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
 
	//Do something with the end results
	FinalPos = tr.endpos;

	if ( tr.m_pEnt )
	{
		if ( tr.m_pEnt->IsNPC() || tr.m_pEnt->IsPlayer()) {
			//DevMsg("Grapple hit an NPC!\n");
			return 0;
		} else {
			//DevMsg("Grapple hit something else!\n");
			//UTIL_ImpactTrace( &tr, DMG_BULLET ); //Puts a mark on what you've hit
			return 1;
		}
	}
}
void CWeaponGrapple::tempBlockWeapon( float seconds ) { //Block the weapon for a certain amount of seconds
	m_flNextSecondaryAttack = gpGlobals->curtime + seconds;
	m_flNextPrimaryAttack = gpGlobals->curtime + seconds;
	SetWeaponIdleTime( gpGlobals->curtime + seconds );
}
void CWeaponGrapple::setHookUpdateTime( float seconds ) { //Set the hook update time
	m_flGrappleNextUpdateTime = gpGlobals->curtime + seconds;
}
void CWeaponGrapple::FireHook( void )
{
	if(GrappleHitSomething()) {
		//washere
		#ifndef CLIENT_DLL
			decideVecAimingHook();
		#endif
		calculateTimeToSurface();
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}
}
void CWeaponGrapple::calculateTimeToSurface( void ) {
	//Velocity variation divided by meters to travel equals the time it would take to hook
	float centerZ = CollisionProp()->WorldSpaceCenter().z;
	float requiredTime;
	Vector totaldelta;
	totaldelta = IntervalDistance( FinalPos.z, centerZ - CollisionProp()->OBBMins().z, centerZ - CollisionProp()->OBBMaxs().z );
	float dist = totaldelta.Length();
	requiredTime = dist/GRAPPLE_AIR_VELOCITY;
	m_secondsToSurface = gpGlobals->curtime + requiredTime;
}
bool CWeaponGrapple::hookIsThere( void ) { //Used to determine if our hook is already at the place it should be
	//Velocity variation divided by meters to travel equals the time it would take to hook
	if(gpGlobals->curtime > m_secondsToSurface) {
		return 1;
	}
	return 0;
}
void CWeaponGrapple::decideVecAimingHook( void ) {
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if(pOwner == NULL || gpGlobals->curtime < m_flNextPrimaryAttack) {
		DevMsg("decideVecAimingHook() Either pOwner is null or your curtime is less than m_flNextPrimaryAttack. \n");
		return;
	}
	if(IsPlayerGrappling()) { //hookIsThere
		DevMsg("decideVecAimingHook() player is grappling, calling movePlayerIntoVector() \n");
		movePlayerIntoVector();
	} else {
		vecAimingRemember = pOwner->GetAutoaimVector( 0 );
		m_playerIsGrappled = true;
		DevMsg("decideVecAimingHook() player is not grappling, remembering vector \n");
	}
}
void CWeaponGrapple::movePlayerIntoVector( void ) {
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if(pOwner == NULL) {
		return;
	}
	if(hookIsThere()) {
		DevMsg("movePlayerIntoVector() called \n");
		QAngle angAiming;
		VectorAngles( vecAimingRemember, angAiming );
		//Update functions
		tryUngrappleOnEnd();
		//End of Update functions
		if ( pOwner->GetWaterLevel() == 3 ) {
			pOwner->SetGroundEntity( NULL ); //TODO: TEST IF THIS IS NEEDED FOR WATER
			pOwner->SetAbsVelocity( vecAimingRemember * GRAPPLE_WATER_VELOCITY );
			//pOwner->SetAbsOrigin( FinalPos );
		} else {
			pOwner->SetGroundEntity( NULL ); //FIXED: Set player to be not on the ground
			pOwner->SetAbsVelocity( vecAimingRemember * GRAPPLE_AIR_VELOCITY );
			//pOwner->SetAbsOrigin( FinalPos );
		}
	}
}
void CWeaponGrapple::tryUngrappleOnEnd( void ) {

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	//Create our trace_t class to hold the end result
	trace_t tr;
	Vector vecAbsBegin;
 
	//Do something with the end results
	Vector delta = FinalPos - vecAbsStart;
	Vector totaldelta;
	float centerZ = CollisionProp()->WorldSpaceCenter().z;
	//delta.z = IntervalDistance( FinalPos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
	totaldelta = IntervalDistance( FinalPos.z, centerZ - CollisionProp()->OBBMins().z, centerZ - CollisionProp()->OBBMaxs().z );
	//float dist = delta.Length();
	float dist = totaldelta.Length();
	//UNDONE
	//if ( dist < 5.0f ) { //what is the point of this?
		//m_playerIsGrappled = false;
		//tempBlockWeapon(3); //Block client interaction with the weapon for 3 seconds so the animations don't get skipped by the client
	//}
	DevMsg("Distance Left %f: vecAbsEnd: %f. \n", dist, FinalPos);
}

void CWeaponGrapple::UpdatePlayerBeam( void )
{
	//We'll do something with the lasers here.
	if(!IsPlayerGrappling()) {
		if(pHook) {
			KillBeam();
		}
	} else {
		MakeBeam();
	}
}

void CWeaponGrapple::MakeBeam( void )
{
	//if (!pHook) {
		DevMsg("MakeBeam called. \n");
		#ifndef CLIENT_DLL
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		CBeam *m_hBeam = CBeam::BeamCreate( HOOK_SPRITE , 2.0f );
		m_hBeam->SetStartPos( pPlayer->Weapon_ShootPosition() );
		m_hBeam->SetEndPos( FinalPos ); 
		m_hBeam->SetEndAttachment( 0 ); //0
		m_hBeam->SetWidth( 0.01f );
		m_hBeam->SetBrightness( 255 ); 
		m_hBeam->SetColor( 255, 255, 255 ); 
		m_hBeam->SetNoise( 0 );
		m_hBeam->AddSpawnFlags( SF_BEAM_TEMPORARY );
		m_hBeam->LiveForTime( 0.002f );
		#endif
		pHook = true;
	//}
}

void CWeaponGrapple::KillBeam( void )
{
	#ifndef CLIENT_DLL
	DevMsg("KillBeam called. \n");
	if ( pHook )
	{
		UTIL_Remove( m_hBeam );
		m_hBeam = NULL;
		pHook = false;
	}
	#endif
}

CWeaponGrapple::CWeaponGrapple()
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;
	m_bFiresUnderwater	= true;
	m_playerIsGrappled = false;
	m_secondsToSurface = 0; //Set it to 0
	pHook = false;
}

void CWeaponGrapple::Spawn( void )
{
	Precache();
	//SetModel( HOOK_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetGravity( 0.05f );
	m_playerIsGrappled = false;
	m_secondsToSurface = 0; //Set it to 0
	pHook = false;
	BaseClass::Spawn();
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponGrapple::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,					ACT_DOD_STAND_AIM_SPADE,				false },
	{ ACT_MP_CROUCH_IDLE,					ACT_DOD_CROUCH_AIM_SPADE,				false },
	{ ACT_MP_PRONE_IDLE,					ACT_DOD_PRONE_AIM_SPADE,				false },

	{ ACT_MP_RUN,							ACT_DOD_RUN_AIM_SPADE,					false },
	{ ACT_MP_WALK,							ACT_DOD_WALK_AIM_SPADE,					false },
	{ ACT_MP_CROUCHWALK,					ACT_DOD_CROUCHWALK_AIM_SPADE,			false },
	{ ACT_MP_PRONE_CRAWL,					ACT_DOD_PRONEWALK_AIM_SPADE,			false },
	{ ACT_SPRINT,							ACT_DOD_SPRINT_AIM_SPADE,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_SPADE,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_SPADE,			false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_PRONE_SPADE,		false },
};

IMPLEMENT_ACTTABLE( CWeaponGrapple );
