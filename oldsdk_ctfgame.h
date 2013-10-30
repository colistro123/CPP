#include "studio.h"

//Game Modes
#define GAMEMODE_CT 0
#define GAMEMODE_CTF 1


//Flag States:
//Attempted to create unknown entity type 
#define FLAG_STATE_BASE 0
#define FLAG_STATE_STOLEN 1
#define FLAG_STATE_DROPPED 2


#if !defined( CLIENT_DLL )
#include "entitylist.h"
#endif

#ifdef CLIENT_DLL
	#define CBaseEntity C_BaseEntity
#endif

#ifdef CLIENT_DLL //Client side code
class CTFClientSide : public CBaseEntity //was CBaseEntity
{
//Network
public:
	DECLARE_CLASS(CTFClientSide, CBaseEntity);	// setup some macros
	DECLARE_CLIENTCLASS();  // make this entity networkable
	DECLARE_DATADESC();
//Functions
	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	void	ClientThink();
	//Constructor, destructor
	CTFClientSide( void );
	virtual ~CTFClientSide( void );
private:
// Networked variables as defined in server class
	int		gameType;
	int		mi6_Flags;
	int		phoenix_Flags;
	int		mi6_Flag_State;
	int		phoenix_Flag_State;
};
#endif

#if defined( GAME_DLL ) //Server side code
class CTFScript : public CBaseEntity //was CBaseEntity
{
public:
	DECLARE_CLASS(CTFScript, CBaseEntity );	// setup some macros
	//Constructor
	CTFScript();

	//Declares
	DECLARE_SERVERCLASS();  // make this entity networkable
	DECLARE_DATADESC();
	//Functions
	void tryToInitializeCTF( void );
	void InitializeCTFGame( void );
	int UpdateTransmitState();
	int	GetGameType( void ){ return gameType; }
	//Destructor
	virtual ~CTFScript();
private:
	unsigned int gType;
	CNetworkVar( int, gameType );
	CNetworkVar( int, mi6_Flags );
	CNetworkVar( int, phoenix_Flags );
	//Flag states: Stolen, captured, etc
	CNetworkVar( int, mi6_Flag_State );
	CNetworkVar( int, phoenix_Flag_State );
};
#endif
