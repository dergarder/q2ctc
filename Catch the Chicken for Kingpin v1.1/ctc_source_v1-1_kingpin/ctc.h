

//
//	Chicken routines found elsewhere
//
extern void NoAmmoWeaponChange (edict_t *ent);
extern edict_t *SelectRandomDeathmatchSpawnPoint (edict_t *ent);
extern edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ent, qboolean team_spawnbase);
extern void	 Touch_Item			(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void	 Weapon_Generic		(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
extern void	 P_ProjectSource	(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
extern void	 debris_die			(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);	// Chicken Egggun

void	 ClientUserinfoChanged (edict_t *ent, char *userinfo);

extern qboolean Chicken_Pickup				(edict_t *ent, edict_t *other);
extern qboolean Chicken_CanPickup			(edict_t *ent, int allow);
extern qboolean Chicken_AllowPowerUp		(edict_t *ent, edict_t *other);
extern qboolean	Chicken_ItemTouch			(edict_t *ent, edict_t *other);
extern qboolean	Chicken_DropMakeTouchable	(edict_t *ent);
extern qboolean Chicken_CanPickupHealth		(edict_t *ent, edict_t *other);
extern qboolean	Chicken_PlayerDie			(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath);
extern qboolean Chicken_TossCheck			(edict_t *ent);
extern qboolean Chicken_Ready				(edict_t *ent);
extern qboolean Chicken_InvUse				(edict_t *ent);
extern qboolean Chicken_InvDrop				(edict_t *ent);

extern char *Chicken_GetModelName	(edict_t *ent);

extern void Chicken_Spawn			(void);
extern void Chicken_Drop			(edict_t *ent,struct gitem_s *item);
extern void Chicken_Weapon			(edict_t *ent);
extern void Chicken_Toss			(edict_t *ent,struct gitem_s *item);
extern void Chicken_GameInit		(void);
extern void Chicken_RunFrameEnd		(edict_t *ent);
extern void Chicken_EndIt			(edict_t *ent);
extern void Chicken_ScoreCheck		(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath);
extern void Chicken_CheckInPlayer	(edict_t *ent);
extern void Chicken_CheckGlow		(edict_t *ent);
extern void Chicken_ShowGun			(edict_t *ent);
extern void Chicken_Kill			(edict_t *ent);
extern void Chicken_Stats			(edict_t *ent);

//
// Model Details Used in CTC
//
typedef struct
{
	char	*playerModel;
	char	*menuText;
	char	*playerGender;
	char	*playerModelPath;
	char	*chickenModel;
	char	*eggGunModel;
} ModelDetails;

// Must match order	{"Red","Blue","Yellow","White","Green","Pink","Aqua","Black"};

/*typedef struct
{
	char	*colour;
	char	*skin;
	char	*icon;
	char	*holdIcon;
	char	*scoreIcon;
	char	*scoreHoldIcon;
	int		score;
	int		players;
	char	menu[MENU_ITEM_LINE_LEN];
} TeamDetails;
*/
//extern TeamDetails	teamDetails[];
extern ModelDetails playerModels[];

typedef struct NextLevel
{
	char				*mapName;
	struct NextLevel	*next;
} LevelName;


#define MAX_TEAMS				4
#define MAX_PLAYER_MODELS		3

//
// MIN/MAX values for number of feather
//
#define MAX_FEATHER_FRAMES		15
#define MIN_FEATHER_FRAMES		1

#define TIME_LOW_TIMEOUT        3 // How low chicken hold timer gets before making sounds

#define	CHICKEN_TIMER_OFF	0
#define	CHICKEN_TIMER_ON	1
//
//	Chicken throw parameters taken from grenade
//
#define CHICKEN_TIMER		2.0
#define CHICKEN_MINSPEED	400
#define CHICKEN_MAXSPEED	600
#define EGG_SPEED			1000	// Speed of egg


//
//	Location and name of files required for CTC
//
#define SPACER					" "
#define SOUND_RANDOM1			"chicken/random1.wav"
#define SOUND_RANDOM2			"chicken/random2.wav"
#define SOUND_RANDOM3			"chicken/random3.wav"
#define SOUND_FIRE_WEAPON		"chicken/fire.wav"
#define	SOUND_READY_TO_THROW	""
#define	SOUND_CHICKEN_ANGRY		""
#define SOUND_CHICKEN_SPAWN		"chicken/respawn.wav"
#define SOUND_CHICKEN_RESPAWN	"chicken/respawn.wav"
#define SOUND_CHICKEN_HURT_25	""
#define SOUND_CHICKEN_HURT_50	""
#define SOUND_CHICKEN_HURT_75	""
#define SOUND_CHICKEN_HURT_100	""
#define SOUND_CHICKEN_DIE		"chicken/chickdie.wav"
#define SOUND_GAME_START		"chicken/start.wav"
#define SOUND_GAME_END			"chicken/end.wav"
#define SOUND_MESSAGE			"chicken/message.wav"
#define SOUND_SWIM				""
#define	SOUND_CHICKEN_SCRATCH	""
#define	SOUND_CHICKEN_PECK		"chicken/peck.wav"
#define	SOUND_OBSERVER			"items/protect3.wav"
#define	SOUND_CHICKEN_PICKUP	"chicken/pickup.wav"

#define	SOUND_EGGGUN_READY		""
#define	SOUND_EGGGUN_FIRE		"chicken/egggun.wav"
#define	SOUND_EGG_SPLAT			"chicken/splat.wav"
#define	SOUND_TIME_UP			"chicken/clock/bell.wav"
#define	SOUND_TIME_LOW			"chicken/clock/tick.wav"

#define SOUND_PRECACHE \
			SOUND_RANDOM1			SPACER \
			SOUND_RANDOM2			SPACER \
			SOUND_RANDOM3			SPACER \
			SOUND_FIRE_WEAPON		SPACER \
			SOUND_READY_TO_THROW	SPACER \
			SOUND_CHICKEN_ANGRY		SPACER \
			SOUND_CHICKEN_SPAWN		SPACER \
			SOUND_CHICKEN_RESPAWN	SPACER \
			SOUND_CHICKEN_DIE		SPACER \
			SOUND_GAME_START		SPACER \
			SOUND_GAME_END			SPACER \
			SOUND_MESSAGE			SPACER \
			SOUND_SWIM				SPACER \
			SOUND_CHICKEN_SCRATCH	SPACER \
			SOUND_CHICKEN_PECK		SPACER \
			SOUND_OBSERVER			SPACER \
			SOUND_CHICKEN_PICKUP    SPACER \
			SOUND_TIME_UP			SPACER \
			SOUND_TIME_LOW			

/*			SOUND_CHICKEN_HURT_25	SPACER \
			SOUND_CHICKEN_HURT_50	SPACER \
			SOUND_CHICKEN_HURT_75	SPACER \
			SOUND_CHICKEN_HURT_100	SPACER \
			SOUND_EGGGUN_READY		SPACER \
			SOUND_EGGGUN_FIRE		SPACER \
			SOUND_EGG_SPLAT			SPACER \
*/

//
// Models used in CTC
//
#define MODEL_WEAPON_THROW		"models/weapons/v_throw/tris.md2"
#define MODEL_WEAPON_NORMAL		"models/weapons/v_chick/tris.mdx"

#define MODEL_CHICKEN_ITEM		"models/chicken/chicken.mdx"
#define MODEL_CHICKEN_FLY		"models/objects/fly_chick/tris.md2"
#define MODEL_FEATHER			"models/objects/feather/tris.md2"

#define ICON_CLOCK				"/pics/i_clock.tga"

//
// Game parameters accessed by quake functions
//
extern int		chickenGame;		// Has the Chicken game started
extern int		allowSmallHealth;	// Can players pickup small health packs
extern int		allowBigHealth;		// Can players pickup big health packs
extern int		allowadrenaline;	// Can players pickup the adrenaline
extern int		normaldamage;		// Normal weapon damage or less
extern int		allowArmour;		// Can players pickup armour
extern int		allowGlow;			// Can Chicken Player Glow
extern int		allowweaponmods;	// Can players pickup weapon mods
extern int		respchickAllowed;	// Is client allowed to respawn the chicken?

extern int		chickenItemIndex;	// Global index of Chicken item
extern struct gitem_s *chickenItem;	// Global Pointer to Chicken Item
