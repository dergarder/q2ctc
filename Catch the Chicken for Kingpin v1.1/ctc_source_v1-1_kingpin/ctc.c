

/**************************************************************************
 Original idea by 

	20/09/2000	Catch the Chicken started. Added standard MOTD stuff

	24/09/2000	Back to the grind and added the model for the chicken to
				items.c and then all the routines etc until the damn
				thing would compile. Now its crashing the DLL :(

	25/09/2000	The DLL bug is now fixed. The chicken model pointer hadn't
				been initialised so it pointed to shit hence the crash.
				But now its crashing again and its the kingpin.exe this
				time.

	26/09/2000	The exe crash was because of modelindex2 in the entity
				_state_t structure but I don't need it anyway :)

	27/09/2000	Spent all night looking at how the models work in KP for
				adding the chicken so everyone can see it in the players
				hand.

	28/09/2000	Adding more routines and calls to get more of CTC working
				properly.

	29/09/2000	Added yet more missing routines to the game and also
				figured out the models for keeping in the CTC folder. At
				long last the remaining bug has been sorted in which when
				throwing the chick it threw 2 and didn't put the weapon
				back. A full beta test is on the cards for tomorrow at
				Warrens. Oh happy days.

	04/10/2000	Damn I haven't been updating this. Right the chicken models
				are all sorted just need the skin from Millymog. The display
				timer is now working a treat and displaying correctly. Don't
				know what is next. :)

	05/10/2000	Enlarged the chicken pickup graphic. Timer now stops when
				game over. Bubbles also rise from the chicken when under
				water.

	06/10/2000	Sorted problem with Ticals vweap not showing, I'd changed
				the name for the chook pickup and it used that to get the 
				model. Also found missing take damage routine and added
				that. Pak file about to be finalised for LAN testing at
				JoLLy's tomorrow.

	08/10/2000	Testing went very well. Things to sort are allow adrenaline,
				burning when you can't take damage (done), armour still goes
				down and weapon power seems too high and needs reducing. This
				will be done with an option to have normal or reduced power.
				Armour problem sorted.
	
	09/10/2000	Adrenaline problem sorted. Health wasn't being checked
				properly as the routine had been changed for health pickup.

	18/10/2000	Added the anti cheat stuff and the spawn fix. Coloured the
				MOTD. Sorted ini file reading. Added pickup chicken point.
				Fuck it, I'm going to beta it now :)

	22/10/2000	Well as expected a few problems have occured. Skin for flying
				chick was missing from PAK for one. Added drop-pickup delay
				so player has to wait before picking up again. Added score-
				board highlight for player holding chook. Added maxscore for
				holding the chook and forces it to be dropped afterwards.
				Sound also plays to denote auto drop. Lava death crash bug &
				held too long crash bug both fixed. Thats it for today.

	17/11/2000	Well after a nice 2 week break in Orlando its finally back
				to doing some coding. Added STDLOGGING at last so now Truz
				can test it :)

	20/11/2000	Made name bigger in scoreboard, added chicken holder kills,
				increased shotgun kick, added mapname to scoreboard, added
				hook command ignore. Added "vote respawn" instead of
				straight forward client respawn, this should stop naughty
				people.

	29/11/2000	Truz has given me a whopping big wish list (as usual) quite a
				few nice things so I'll leave these until my Xmas break when I've
				got more time. There's only 1 bug seen so far which I know how to fix
				but its not major so I'll leave it until v1.1.

	08/04/2001	Well later than planned but starting to fix/add things.
				1) Chicken cannot be picked up until more than 1 person in game
				2) Scoreboard names left justified.
				3) No respawn messages on dedicated server when empty
				4) Removed the stdlog vars from the serverinfo

**************************************************************************/

#include "g_local.h"
#include "frames.h"
#include "ctc.h"
#include "io.h"
#include "stdlog.h"    // StdLog
#include "gslog.h"    // StdLog

static void Chicken_RemoveFromInventory(void);
static void Chicken_Setup(edict_t *chicken, edict_t *owner);
static void Chicken_Drop_Temp(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
static void Chicken_Think (edict_t *self);
static void Chicken_FlyThink (edict_t *self);
static void Chicken_RandomSound(edict_t *self);
static void Chicken_Float(edict_t *self);
static void Chicken_AdvanceFrame(edict_t *self);
static void Chicken_ClockStart (edict_t *ent);
static void Chicken_ClockThink (edict_t *timer);
static void Chicken_ThrowFeather (edict_t *self, int count);
static void Chicken_Fire(edict_t *ent);
static void Chicken_EnsureExists(void);
static void DisplayHeldOverMessage( edict_t *chookrustler );
static int  PlayersInGame( void );

static qboolean Chicken_CheckAutoRespawn(edict_t *self);
static edict_t *Chicken_FindSpawnSpot(edict_t *ent);

extern int QweryClipIndex (gitem_t *item);

// Variables and defines go here.
#define	INI_FILE	"ctc.ini"

int			respawnTime			= 10;			// Game Time when chicken spawned
int			chickenGame			= true;			// Has the Chicken game started

int			allowBigHealth		= false; 		// Can players pickup large health items
int			allowSmallHealth	= false; 		// Can players pickup small health items
int			allowArmour			= false;		// Can players pickup armour
int			allowweaponmods		= false;		// Can players pickup weapon mods
int			allowadrenaline		= false;		// Can players pickup the adrenaline
int			normaldamage		= true;			// Normal weapon damage else lesser damage
int			allowGlow			= 3;			// Chicken and player glow colour
int			chickGlow			= false;		// Chicken standing glow
int			canDrop				= true;			// Is the options menu up
int			scoreOnDeath		= true;			// Do you score for killing person with chicken
int			scoreonpickup		= false;		// Do you score for picking up the chicken
int			scorePeriod			= 5;			// How long to hold chicken before scoring
int			dropDelay           = 10;			// How long you have to hold onto chicken before u can drop it
int			feathers            = 5;			// Chance of throwing random feather
int			spawnDelay			= 10;			// Chicken cannot be respawned for spawnDelay seconds
int			stdLogging			= false;
int			randomSpawn			= true;			// Does chicken respawn at random place
int			respchickAllowed    = false;		// Is menu allowed up
int			autoRespawn			= false;		// Allow Auto Respawn of chicken
int			autorespawntime		= 60;			// How often to auto respawn
int			cantTouchDelay		= 0;			// How long after throwing chicken can player pick it up
int			maxHoldScore		= 10;			// Maximum score player can get while holding chicken

int			chickenItemIndex	= 0;			// Index of chicken item in item list
int			teams				= 0;			// How many teams can be in tctc

struct gitem_s *chickenItem	    = NULL;			// Pointer to chicken item
struct gitem_s *eggGunItem	    = NULL;			// Pointer to egg gun item

static qboolean	wasInWater		= false;		// Was chicken in the water
static char gameStatusString[]  = "start game             "; // Must be big enough to store any menu text

#ifdef STD_LOGGING	
qboolean	loggingStarted		= false;		// Has Std logging commenced
#endif

//
// Ini file options
//
typedef struct
{
	char	*ident;
	int		*variable;
	int		MinVariable;
	int		MaxVariable;
	int		DefaultVariable;
} INI_OPTION;

INI_OPTION	option[] = 

//	Normal Options

		{	{"bighealth",		&allowBigHealth,	0,1,0},
			{"smallhealth",		&allowSmallHealth,	0,1,0},
			{"armor",			&allowArmour,		0,1,0},
			{"fragscore",		&scoreOnDeath,		0,1,1},
			{"pickupscore",		&scoreonpickup,		0,1,0},
			{"stdlog",			&stdLogging,		0,1,0},	// Dont change ident name
			{"allowresp",		&respchickAllowed,	0,1,0},	
			{"autorespawn",		&autoRespawn,		0,1,1},	
			{"autorespawntime", &autorespawntime,	30,120,60},	
			{"scoretime",		&scorePeriod,		3,60,5},
			{"droptime",		&dropDelay,			5,15,10},
			{"maxholdscore",    &maxHoldScore,		0,20,10},
			{"glow",			&allowGlow,			0,4,4},
			{"weapmod",			&allowweaponmods,	0,1,0},
			{"adrena",			&allowadrenaline,	0,1,0},
			{"normalweapons",	&normaldamage,		0,1,1},
			{"chickglow",		&chickGlow,			0,1,0},
			{"droppable",		&canDrop,			0,1,1},
			{"feathers",		&feathers,			3,10,5},
			{"respawntime",		&spawnDelay,		4,15,10},
			{"notouchdelay",    &cantTouchDelay,	0,10,0},
			{"randomspawn",		&randomSpawn,		0,1,1}
		};

//
// CTC Death messages
//
char *killedSelf[] =
		{	{"%s dies\n"									}};
char *killerKilled[] =
		{	{"%s kills %s the chicken rustler\n"			},
			{"%s plucks %s's chicken\n"						},
			{"%s turns %s into nuggets\n"					},
			{"%s has %s with 11 secret herbs and spices\n"	},
			{"%s roasts %s\n"								},
			{"%s plays mash the chicken with %s\n"			}};
char *killer[] =
		{	{"%s frees the chicken's soul\n"				},
			{"%s kills the chicken rustler\n"				},
			{"%s forces release of the chicken\n"			},
			{"%s administers a fatal plucking\n"			},
			{"%s makes a sacrifice to the chicken god\n"	},
			{"%s skews some dinner\n"						},
			{"%s is now fit to enter chicken heaven\n"		},
			{"The chicken god smiles upon %s\n"				}};
char *killed[] =
		{	{"Mc %s Nugggets to go!\n"						},
			{"%s dies for the chicken god\n"				},
			{"%s has been judged by the chicken god\n"		}};

//*************************************************************************

typedef struct   // Message of the Day
	{
	char textline[100];
	} MOTD_t;

	MOTD_t	MOTD[20];

#define MAX_OPTIONS (sizeof(option)/sizeof(option[0]))
#define MOTD_lines		2

/*************************************************************************
/*	Chicken initalization for first time game is run
/************************************************************************/
void Chicken_GameInit()
	{
	static qboolean firstTime = true;							

	if (firstTime)												
		{
//		eggGunItem			= FindItem("EggGun");				
//		eggGunItemIndex		= ITEM_INDEX(FindItem("EggGun"));	

		chickenItem			= FindItem("Chicken");				
		chickenItemIndex	= ITEM_INDEX(FindItem("Chicken"));	

//		Chicken_MenuCreate();					
//		Chicken_ReadIni();

//		if (levelCycling)
//			Chicken_ReadMapList();

//		if (MOTDDuration != 0)
//			Chicken_MOTDLoad();

		if (teams)
			{
//			Chicken_MenuTCTCCreate();
//			Chicken_TeamMenuCreate();
//			Chicken_PlayerSelectMenuCreate();
			}

//		Chicken_CreateStatusBar();

		firstTime = false;										
		}															
	}

/*************************************************************************
/*	Spawn a chicken
/************************************************************************/
void Chicken_Spawn()
	{
	edict_t		*chicken	= NULL;
	edict_t		*spot		= NULL;
	edict_t		*e			= NULL;
	int			i;

	if (!deathmatch->value)
		return;

	// Remove any abnormalies. This shouldn't have been called otherwise
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
		{
		if (!e->inuse)
			continue;

		// Remove chicken item from game
		if (!strcmp(e->classname, "item_chicken"))
			{
			e->nextthink	= 0;
			e->health		= 0;
			G_FreeEdict (e);
			}
		}

	Chicken_RemoveFromInventory();

	if ((chicken = G_Spawn()) != NULL)
		{
		Chicken_Setup(chicken, NULL);

		spot = Chicken_FindSpawnSpot(chicken);

		chicken->nextthink		= level.time + FRAMETIME;
		chicken->s.frame		= FRAME_stand01;
		chicken->owner			= NULL;

		VectorCopy (spot->s.origin, chicken->s.origin);
		chicken->s.origin[2] += 2;
		VectorCopy (spot->s.angles, chicken->s.angles);
		M_droptofloor(chicken);
		gi.linkentity (chicken);

		// Send Spawn Effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (chicken-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (chicken->s.origin, MULTICAST_PVS);

		if (strlen(SOUND_CHICKEN_SPAWN))
			gi.sound(chicken, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_SPAWN), 1, ATTN_NONE, 0);

		chickenGame = true;
		wasInWater  = false;

		respawnTime	= level.time + spawnDelay;
		sprintf(gameStatusString, "end game");
		}
	else
		gi.dprintf("Respawn chicken failed.");
	}

/************************************************************************
//	Setup new chicken defaults
/***********************************************************************/
static void Chicken_Setup(edict_t *chicken, edict_t *owner)
	{
//	teamWithChicken			= -1;
	chicken->classname		= chickenItem->classname;
	chicken->item			= chickenItem;
	chicken->spawnflags		= DROPPED_PLAYER_ITEM;
	chicken->s.effects		= 0; //chickenItem->world_model_flags;
	
	// BEGIN CTC - Make the chicken glow to assist finding.
	if (chickGlow)
		chicken->s.effects |= EF_HYPERBLASTER;
	// END

	chicken->s.renderfx		= 0;
	VectorSet (chicken->mins, -7, -7, -15);
	VectorSet (chicken->maxs,  7,  7,   0);
	gi.setmodel (chicken, MODEL_CHICKEN_ITEM);
	chicken->solid			= SOLID_TRIGGER;
	chicken->movetype		= MOVETYPE_STEP;
	chicken->enemy			= NULL;
	chicken->nextRespawn	= level.time + autorespawntime;
	chicken->s.frame		= 0;
	chicken->touch			= Chicken_Drop_Temp;
	chicken->owner			= owner;
	chicken->think			= Chicken_Think; 

	chicken->teleport_time  = level.time + cantTouchDelay;

	// Mask Chicken so that it can stand on spawn points when dropped
	chicken->clipmask		= MASK_PLAYERSOLID;

	wasInWater  = false;
	}

/****************************************************************************************
/*	Main Chicken thinking routine calls all action function to make chicken do stuff
/***************************************************************************************/
static void Chicken_Think (edict_t *self)
	{
	if (self)
		{
		//
		// Check for auto respawn if allowed
		//
		if (Chicken_CheckAutoRespawn(self))
			return;

		//
		// Sound the chicken squawk
		//
		Chicken_RandomSound(self);

		// 
		// Check and Make Chicken Float if in Water
		// 
		Chicken_Float(self);

		//
		// Work out what chicken is in
		//
		self->watertype = gi.pointcontents (self->s.origin);

		//
		// If chicken is in the lava or slime force respawn
		//
		if (self->watertype & (CONTENTS_SLIME|CONTENTS_LAVA))
			{
			Chicken_Spawn();
			gi.bprintf(PRINT_HIGH, "Chicken Respawned out of danger\n");	
			}

		//
		// Do next Animation Frame
		//
		Chicken_AdvanceFrame(self);

		self->nextthink		= level.time + FRAMETIME;
		gi.linkentity (self);
		}
		else
			gi.dprintf("Chicken_Think NULL");
	}

/************************************************************************
/*	Check if chicken should auto respawn
/***********************************************************************/

static qboolean Chicken_CheckAutoRespawn(edict_t *self)
	{

	if (autoRespawn && level.time > self->nextRespawn && PlayersInGame() > 0)
		{
		Chicken_Spawn();
		gi.bprintf(PRINT_HIGH, "Chicken has auto respawned\n"); 
		return true;
		}
	return false;
	}

/************************************************************************
/*	Fallback function to guarentee that chicken exists somewhere
/***********************************************************************/
static void Chicken_EnsureExists()
	{
	static long checkNext = 0;
	
	if (deathmatch->value && chickenGame && checkNext < level.time)
		{
		checkNext = level.time + 10;
		if (chickenGame)
			{
			edict_t		*e  = NULL;
			int	i, foundIt	= false;
			int			contents;

//			clientCount = 0;
			// Ensure chicken exists
			for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
				{
				if (!e->inuse)
					continue;

				// Keep a client count so for dedicated servers we dont display stuff to logs
//				if (e->client) clientCount++;

				if ((e->client && e->client->pers.inventory[chickenItemIndex] > 0 && !e->client->isThrowing))
					foundIt = true;

				// Check for item chicken not in world or water	
				if (!strcmp(e->classname, "item_chicken"))
					{
					contents = gi.pointcontents (e->s.origin);
					if (!(contents & (CONTENTS_SOLID|CONTENTS_WINDOW)))
						foundIt = true;
					}
	
				// Just ensure that people can score
				if (teams && e->client)
					{
					if (e->client->pers.inventory[chickenItemIndex] == 0 && e->client->holdScore)
						{
						gi.cprintf(e, PRINT_HIGH, "Sorry you couldnt score. Try now\n");
						e->client->holdScore = 0;
						}
					}
				}

			if (!foundIt)
				{
				Chicken_Spawn();
				gi.bprintf(PRINT_HIGH, "Chicken had lost its way and was respawned\n"); 
				}
			}
		}
	}

/***********************************************************************
/*	Cleanup routine for when CTC Ends
/**********************************************************************/
void Chicken_EndIt(edict_t *ent)
	{
	edict_t *e	  = NULL;
	int i;

	Chicken_RemoveFromInventory();

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
		{
		if (!e->inuse)
			continue;

		if (e->client)	// Pull everyone out of there menu's. Dont have to just want too..
			e->client->displayMenu = false;

		// Remove anything that could be laying around from the game
		if (!strcmp(e->classname, "item_chicken") ||
		    !strcmp(e->classname, "feather")      ||
		    !strcmp(e->classname, "chicken_timer"))
		{
			e->nextthink	= 0;
			e->health		= 0;
			G_FreeEdict (e);
		}
	}

#ifdef STD_LOGGING	
	if (loggingStarted)
		{
		loggingStarted = false;
		sl_GameEnd( &gi, level );	// StdLog - Mark Davies
		}
#endif

	// Reset globals
	chickenGame			= false;
	sprintf(gameStatusString,"start game");

	if (strlen(SOUND_GAME_END))
		gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_GAME_END),  1, ATTN_NORM, 0);	

	// Back to Deathmatch
	}

/***********************************************************************/
/* These routines are to minimize changes to the original code.
/***********************************************************************/

void Chicken_RunFrameEnd(edict_t *ent)
	{
	Chicken_EnsureExists();
	if (chickenGame && teams == 0 && ((int)(dmflags->value) & (DF_MODELTEAMS /*| DF_SKINTEAMS*/)))
		{
		Chicken_EndIt(ent);
		gi.bprintf(PRINT_HIGH, "Catch the Chicken must be started with teamplay\n");
		}
	}

/************************************************************************
/*	Make chicken make some noise so it can be located
/***********************************************************************/

static void Chicken_RandomSound(edict_t *self)
	{
	static int lastPlayedFrame = 0;

	if (self)
		{
		if (lastPlayedFrame >= 50)
			{
			float playSound		= random();
			char  *soundToPlay	= "";

			if (playSound > 0.50 && playSound <= 0.66)
				soundToPlay = SOUND_RANDOM1;
			else if (playSound > 0.66 && playSound <= 0.82)
				soundToPlay = SOUND_RANDOM2;
			else if (playSound > 0.82)
				soundToPlay = SOUND_RANDOM3;

			if (strlen(soundToPlay))
				gi.sound(self, CHAN_AUTO, gi.soundindex(soundToPlay),  1, ATTN_NORM, 0);	

			lastPlayedFrame = 0;
			}
		else
			lastPlayedFrame++;
		}
		else
			gi.dprintf("Chicken_RandomSound NULL");
	}

/************************************************************************
/*	Makes chicken seem to float on water. 
/***********************************************************************/
static void Chicken_Float(edict_t *self)
	{
	qboolean		swimming	= false;
	qboolean		Showbubbles = false;
	static qboolean	resetBob	= true;
	vec3_t			headLoc;
	
	if (self)
		{
		if (self->watertype & CONTENTS_WATER) 
			{
			vec3_t	chickSize = {  0,  0, 11};

			wasInWater = true;
			resetBob   = true;
			Showbubbles = true;

			self->s.angles[0] = 0;
			self->s.angles[1] = 0;

			VectorAdd (self->s.origin, chickSize, headLoc);

			if (!(gi.pointcontents (headLoc) & (CONTENTS_SOLID | CONTENTS_WINDOW)))
				{
				self->s.origin[2] += 2;
				gi.linkentity (self);
				}
			}
		else
			{
			// If chicken is no longer in water and he was then make him bob
			if (wasInWater)
				{
				// Stop him from falling back into the water
				self->movetype = MOVETYPE_NONE;

				if (resetBob)
					{
					self->s.angles[0] = 0;
					self->s.angles[1] = 0;
					resetBob = false;
					}
				else
					{
					// Start the bobbing effect
					self->s.angles[0] += (10.0 * crandom());
					self->s.angles[1] += (10.0 * crandom());
					resetBob = true;
					}
				}
			else
				{
				resetBob = true;
				}
			}
		}
	else
		gi.dprintf("Chicken_Float NULL");

	//
	// Added the bubbles because they look nice. :)
	if (Showbubbles)
		{
		vec3_t	pos;
		trace_t	tr;
		vec3_t	dir, water_start, Bubbletop;
		vec3_t	Bubbleheight = {  5,  0, 32};

		tr = gi.trace (self->s.origin, NULL, NULL, headLoc, self, MASK_SHOT );
//		VectorSubtract (tr.endpos, water_start, dir);
		
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		VectorAdd (self->s.origin, Bubbleheight, Bubbletop);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
//		gi.WritePosition (water_start);
//		gi.WritePosition (tr.endpos);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (Bubbletop);
		gi.multicast (pos, MULTICAST_PVS);
		}

	}

/************************************************************************
/*	Removes chicken from a players inventory
/***********************************************************************/

static void Chicken_RemoveFromInventory()
	{
	//	Removes all timers. Called when no one has the chicken anymore
	edict_t *e	  = NULL;
	int i;

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
		{
		if (!e->inuse) continue;

		// Remove all timers
		if (!strcmp(e->classname, "chicken_timer"))
			{
			e->nextthink = e->health = 0;
			G_FreeEdict (e);
			}

		if (e->client)
			{
//			if (e->client->pers.inventory[eggGunItemIndex] > 0) 
//				{
//				e->client->pers.inventory[eggGunItemIndex]  = 0;

				// Check for dead players, observer or camera players
//				if (e->s.modelindex2 != 0)
//					{
//					NoAmmoWeaponChange(e);
//					e->client->pers.selected_item = ITEM_INDEX(e->client->pers.weapon);	
//					}
//				}

			// Making assumption that if one player throws chicken to another the catching player wont be able
			// to throw it back before the throwers inventory has been make void of chicken Item
			if (e->client->pers.inventory[chickenItemIndex] > 0)
				{
				// Cleanup player just since player was in middle of throw animation

				// Set chicken item model back before we forget
				// Chicken_Fire takes care of grenade_time and weapon changing
				chickenItem->view_model  = MODEL_WEAPON_NORMAL;
				e->client->isThrowing  = false;

				// Set grenade time to zero else exploding chicken
				e->client->grenade_time = 0;

				// Allow player to score again
				e->client->holdScore = 0;

				// Get ride of all chickens. Should only ever be 1 but set it to 0 to make sure
				e->client->pers.inventory[chickenItemIndex] = 0;

				NoAmmoWeaponChange(e);
//				if (e->s.modelindex2 != 0) // Check for dead players, observer or camera players
					ChangeWeapon(e);	// Force the change of weapon NOW!

				e->client->ps.gunindex = gi.modelindex(e->client->pers.weapon->view_model);
				e->client->ps.gunframe = 0;
				e->client->pers.selected_item = ITEM_INDEX(e->client->pers.weapon);
				}
			}
		}
	}

/*************************************************************************************
/*	Change the weapon being viewed by the world.
/************************************************************************************/
void Chicken_ShowGun(edict_t *ent)
	{
	char heldmodel[128];
	char skin[128];
	int len, i;
	int gotSpecial = ent->client->pers.inventory[chickenItemIndex];

	if (!ent->client->pers.weapon)
		return;

/*	strcpy(heldmodel, "#");
	strcat(heldmodel, ent->client->pers.weapon->icon);	
	strcat(heldmodel, ".mdx");
	n = (gi.modelindex(heldmodel) - vwep_index) << 8;

	gi.dprintf("path1 %s\n",heldmodel);

	ent->s.skinnum &= 0xFF;
	ent->s.skinnum |= n;*/

	// set visible model
	if (ent->s.modelindex == 255)
		{
		if (ent->client->pers.weapon)
			i = (((QweryClipIndex(ent->client->pers.weapon)+1) & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
		}

	if (chickenGame)
		{
		strcpy(skin, Info_ValueForKey (ent->client->pers.userinfo, "skin"));

		for(len = 0; skin[len]; len++)
			{
			if(skin[len] == '/')
				{
				skin[len] = '\0';
				break;
				}
			}

		// While we are here if we havent tested for the chicken model check now
		if (ent->client->gotChickenModel == -1)
			{
			char model[128];

//			strcpy(model, "main\\players\\");
			strcpy(model, "ctc\\players\\");
			strcat(model, skin);
			strcat(model, "\\w_chicken.mdx");

			if (_access(model, 2) == 0)
				ent->client->gotChickenModel = 1;
			else
				ent->client->gotChickenModel = 0;
			}

		strcpy(heldmodel, "players/");
		strcat(heldmodel, skin);
		strcat(heldmodel, "/");
		strcat(heldmodel, ent->client->pers.weapon->icon);	
		strcat(heldmodel, ".mdx");

		if (ent->client->pers.inventory[chickenItemIndex] > 0)
			{
			if (ent->client->gotChickenModel > 0)
				{
//				ent->s.modelindex2 = gi.modelindex(heldmodel);
				}
			else
				{
				int			currentModel = 0;
				char		*model = NULL;

				model = Chicken_GetModelName(ent);

				// Go through list of possible player models and display appropriate model
				// If player does not have the correct model use first one in list regardless
//				do 
//					{
//					if (!strcmp(model, playerModels[currentModel].playerModel))
//						break;
//					currentModel++;
//					}
//				while (currentModel < MAX_PLAYER_MODELS);

//				if (currentModel >= MAX_PLAYER_MODELS)
//					currentModel = 0;

//				strcpy(heldmodel, playerModels[currentModel].chickenModel);
//				ent->s.modelindex2 = gi.modelindex(heldmodel);
				}

//			if (ent->s.modelindex2 == 0)
//				ent->s.modelindex2 = 255;

			return;
			}
		}
	}

/*************************************************************************************
/*	Each server frame update any action chicken has to do
/************************************************************************************/

static void Chicken_AdvanceFrame(edict_t *self)
	{
	static qboolean wasSwimming = false;

	// If chicken was in water then make him swim
	if (wasInWater)
		{
		if (wasSwimming == false)
			{
			wasSwimming = true;
			self->s.frame = FRAME_swim01;
			if (strlen(SOUND_SWIM))
				gi.sound(self, CHAN_WEAPON, gi.soundindex(SOUND_SWIM),  1, ATTN_NORM, 0);	
			}
		else
			self->s.frame++;

		if (self->s.frame > FRAME_swim07)
			self->s.frame = FRAME_swim01;
		}
	else
		{
		if (wasSwimming == true)
			{
			wasSwimming = false;
			self->s.frame = FRAME_stand01;
			}

		// If we have finished our previous animation do something different
		if (self->s.frame == FRAME_stand01)
			{	
			int nextAction = (random()*20.0);

			// Add random effect
			switch(nextAction)
				{
				case 0:  case 1:  case 2:
				case 3:  case 4:  case 5:
				case 6:
					// Stand around for a while
					break;

				case 7:	 case 8:  case 9:
				case 10: case 11:
					if (strlen(SOUND_CHICKEN_PECK))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_PECK),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_peck01;
					break;

				case 12:  case 13:  case 14:
				case 15:
					if (strlen(SOUND_CHICKEN_SCRATCH))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_SCRATCH),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_scr01;
					break;

				case 16:
					self->s.frame = FRAME_look01;
					break;

				case 17:
					self->s.frame = FRAME_wob01;
					break;

				case 18:
					self->s.frame = FRAME_walkr01;
					self->ideal_yaw -= 30;
					self->yaw_speed = 30.0/10.0;
					break;

				case 19:
					self->s.frame = FRAME_walkl01;
					self->ideal_yaw += 30;
					self->yaw_speed = 30.0/10.0;
					break;
				}
			}
		else
			{
			if (self->s.frame == FRAME_peck10 || self->s.frame == FRAME_wob25  ||
				self->s.frame == FRAME_scr23  || self->s.frame == FRAME_look23)
				self->s.frame = FRAME_stand01;
			else
				{
				// Make Chicken Peck after any change of angle
				if (self->s.frame == FRAME_walkl10  || self->s.frame == FRAME_walkr10)
					{
					if (strlen(SOUND_CHICKEN_PECK))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_PECK),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_peck01;
					}
				else 
					{
					if (self->s.frame >= FRAME_walkl01 || self->s.frame <= FRAME_walkl10 ||
						self->s.frame >= FRAME_walkr01 || self->s.frame <= FRAME_walkr10)
						M_ChangeYaw (self);

					self->s.frame++;
					}
				}
			}
		}
	}

/*************************************************************************************
//	This function is to prevent player from picking up a freshly dropped chicken
/************************************************************************************/
static void Chicken_Drop_Temp(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner && ent->teleport_time > level.time)
		{
		if (ent->owner && ent->owner->client && ent->owner->client->nextInformTime < level.time)
			{
			gi.cprintf (ent->owner, PRINT_HIGH, "Can't pickup chicken yet\n");
			ent->owner->client->nextInformTime = level.time + 4.0;
			}
		return;
		}
	
	Touch_Item (ent, other, plane, surf);
	}

/*************************************************************************************
/*	Pickup chicken and place in players inventory
/************************************************************************************/
qboolean Chicken_Pickup(edict_t *ent, edict_t *other)
	{	
	char	message1[]="********************************\n\n"
					   "No points will score until\n\n"
					   "another player arrives.\n\n"
					   "********************************";

	// Don't allow pickup when only 1 person in the server.
	if (PlayersInGame()==1)
		{
		gi.centerprintf (other, message1);
		return false;
		}

	if (ent && ent->item && other && other->client)
		{
		Chicken_RemoveFromInventory();

		gi.bprintf(PRINT_HIGH, "%s's picked up the Chicken\n", other->client->pers.netname);

		if (other->flags & FL_GODMODE)
			{
			other->flags ^= FL_GODMODE;
			gi.cprintf(other, PRINT_HIGH, "godmode OFF (Chickens don't like cheats)\n");
			}

		// Add it to the inventory
		other->client->pers.inventory[chickenItemIndex] = 1;
		other->client->newweapon	= chickenItem;
		ent->enemy					= other;

		// Make weapon change straight away. Had problems with firing weapons
		if (other->client->ammo_index)
			other->client->ammo_index = 0;

		// Set when player can drop chicken. Is overriden if dropping not allowed
		other->client->chickenTimer = level.time + dropDelay;

		// Set players gun model
		Chicken_ShowGun(other);

		// Start the timer
		Chicken_ClockStart (other);

		if (strlen(SOUND_CHICKEN_PICKUP))
			gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_PICKUP),  1, ATTN_NORM, 0);	

		if (teams)
			{
//			teamWithChicken = other->client->team;
//			Chicken_TeamReadyEggGun(teamWithChicken);
			}

		// Does the player get a point for picking up the chook?
		if (scoreonpickup)
			other->client->resp.score++;
		}
	else
		gi.dprintf("Chicken_Pickup NULL");

	return true;
	}

/************************************************************************
/*	Checks if player can have powerups when holding chicken
/***********************************************************************/
qboolean Chicken_AllowPowerUp(edict_t *ent, edict_t *other)
	{
	if (chickenGame && other->client && other->client->pers.inventory[chickenItemIndex] > 0 && !other->client->isThrowing)
		{
//		if (!allowInvulnerable && !strcmp(ent->item->classname, "item_invulnerability"))
//			return false;

		if (!strcmp(ent->item->classname, "item_quad"))
			return false;
		}
	return true;
	}

qboolean Chicken_CanPickup(edict_t *ent, int allow)
	{
	if (deathmatch->value && chickenGame && ent->client &&
		ent->client->pers.inventory[chickenItemIndex] > 0 &&
		!ent->client->isThrowing && !allow)
		return false;

	return true;
	}

qboolean Chicken_ItemTouch(edict_t *ent, edict_t *other)
	{
	if (!strcmp(ent->classname, "item_chicken"))
		{
		gi.sound(other, CHAN_ITEM + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_PICKUP), 1, ATTN_NONE, 0);	
		G_FreeEdict (ent);
		return true;
		}
	return false;
	}

qboolean Chicken_DropMakeTouchable(edict_t *ent)
	{
	if (!strcmp(ent->classname, "item_chicken"))
		{	
		ent->nextthink	= level.time + FRAMETIME;
		ent->think		= Chicken_Think;
		return true;
		}	
	return false;
	}

qboolean Chicken_CanPickupHealth(edict_t *ent, edict_t *other)
	{
	if (ent->count == 2)
		{
		if (!Chicken_CanPickup(other, allowSmallHealth)) 
			return false;
		}
	else
		if (!Chicken_CanPickup(other, allowBigHealth)) 
			return false; 

	return true;
	}

qboolean Chicken_TossCheck(edict_t *ent)
	{
	// If you have the chicken but have already throw it dont drop another
	if (deathmatch->value && chickenGame && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		{
		if (!ent->client->isThrowing)
			Chicken_Toss(ent,chickenItem);

		return true;
		}
	return false;
	}

qboolean Chicken_Ready(edict_t *ent)
	{
	if (ent->client->pers.inventory[chickenItemIndex] > 0)
		{
		ent->client->newweapon = chickenItem;
		return true;
		}

//	if (ent->client->pers.inventory[eggGunItemIndex] > 0)
//		{
//		ent->client->newweapon = eggGunItem;
//		return true;
//		}

	return false;
	}

qboolean Chicken_InvUse(edict_t *ent)
	{
	/*
	if (deathmatch->value && ent->client && Chicken_ShowMenu(ent))
		{
		Chicken_MenuItemSelect(ent, ent->client->currentItem->itemId);
		return false;
		}*/
	return true;
	}

qboolean Chicken_InvDrop(edict_t *ent)
	{
	if (deathmatch->value && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		{
		chickenItem->drop (ent, chickenItem);
		return true;
		}

//	if (ent->client && ent->client->pers.inventory[eggGunItemIndex] > 0)
//		return true;

	return false;
	}

void Chicken_Kill(edict_t *ent)
	{
	// Set to DEAD_DYING so that effects will be turned off
	ent->solid		= SOLID_BBOX;
	ent->svflags	&= ~SVF_NOCLIENT;
	ent->deadflag	= DEAD_DYING;
	ent->s.skinnum	= ent - g_edicts - 1;
	ent->s.effects  = 0;
	}

void Chicken_CheckGlow(edict_t *ent)
	{
	if (deathmatch->value && chickenGame && allowGlow && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)  
		{
		switch(allowGlow)
			{
			case 2:	
				ent->s.effects |= EF_FLAG1;
				break;

			case 3:	
				ent->s.effects |= EF_FLAG2;
				break;

			case 4:	
				ent->s.effects |= EF_PLASMA;
				break;

			default:
				ent->s.effects |= EF_HYPERBLASTER;
			}
		}
	}

//	Get clients model name
char *Chicken_GetModelName(edict_t *ent)
	{
	int				len;
	static	char	model[128];
	char			*skin = Info_ValueForKey(ent->client->pers.userinfo, "skin");

	// Fetch back model of player
	for(len = 0; *(skin+len); len++)
		{
		if(*(skin+len) == '/')
			{
			model[len] = '\0';
			break;
			}
		else 
			model[len] = *(skin+len);
		}

	return (model);
	}

/************************************************************************
/*	You guest it when a player dies this is called. Puts up lame messages
/***********************************************************************/
qboolean Chicken_PlayerDie(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath)
	{
	if (chickenGame)
		{
		// Cleanup player just in case player was in middle of throw animation

		// Set chicken item model back before we forget
		// Chicken_Fire takes care of grenade_time and weapon changing
		chickenItem->view_model  = MODEL_WEAPON_NORMAL;

		self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
		self->client->ps.gunframe = 0;

		// Set grenade time to zero else exploding chicken
		self->client->grenade_time = 0;

		// Allow player to score again
		self->client->holdScore = 0;

		if (strlen(SOUND_CHICKEN_DIE))
			gi.sound(self, CHAN_WEAPON, gi.soundindex(SOUND_CHICKEN_DIE),  1, ATTN_NORM, 0);	

		if (self != attacker)
			{
			if (self && self->client)
				{
				self->enemy = attacker;
				if (attacker && attacker->client)
					{
					float which = random();

					if (which < 0.50)
						{
						int msgOff = (int)(((float)sizeof(killerKilled) / sizeof(char *)) * random());
						gi.bprintf (PRINT_MEDIUM, killerKilled[(int)msgOff], attacker->client->pers.netname, self->client->pers.netname);
						}
					if (which >= 0.50)
						{
						int msgOff = (int)(((float)sizeof(killer) / sizeof(char *)) * random());
						gi.bprintf (PRINT_MEDIUM,killer[(int)msgOff], attacker->client->pers.netname);
						}

					}
				else
					{
					int msgOff = (int)(((float)sizeof(killed) / sizeof(char *)) * random());
					gi.bprintf (PRINT_MEDIUM,killed[(int)msgOff], self->client->pers.netname);
					}
				}
			else
				gi.dprintf("Chicken_Die NULL 1");
			}
		else
			{
			if (self)
				{
				int msgOff = (int)(((float)sizeof(killedSelf) / sizeof(char *)) * random());

				self->enemy = NULL;
				gi.bprintf (PRINT_MEDIUM,killedSelf[(int)msgOff], self->client->pers.netname);
				}
			else
				gi.dprintf("Chicken_Die NULL 2");
			}

		return true;
		}
	else
		return false;
	}

/************************************************************************
/*	You guessed it when a player dies this is called. Puts up lame messages
/***********************************************************************/
void Chicken_ScoreCheck(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath)
	{

	if (scoreOnDeath && self->client->pers.inventory[chickenItemIndex] > 0 &&
		!self->client->isThrowing && attacker && attacker->client)
		{
		// If player gets score for killing chicken dude then give it
		attacker->client->resp.score++;
		attacker->client->resp.kills++;

#ifdef STD_LOGGING
			{
			char *pWeaponName = NULL;

			// self->enemy = attacker;
			if( attacker && attacker->client )
				{
				switch (meansOfDeath)
					{
					case MOD_SHOTGUN:
					case MOD_GRENADE:
					case MOD_G_SPLASH:
					case MOD_R_SPLASH:
					case MOD_BFG_LASER:
					case MOD_BFG_BLAST:
					case MOD_BFG_EFFECT:
					case MOD_HANDGRENADE:
					case MOD_HG_SPLASH:
					case MOD_HELD_GRENADE:
					// BEGIN CTC - Kingpin weapons
					case MOD_CROWBAR:
					case MOD_PISTOL:
					case MOD_FLAMETHROWER:
					case MOD_BARMACHINEGUN:
					// END

					default:
						{
						// Get weapon name, being very careful - mdavies
						// This weapon may not have been the weapon used if the weapon was changed before the death
						pWeaponName = (NULL != attacker->client->pers.weapon)?(attacker->client->pers.weapon->pickup_name):(NULL);
						break;
						}

					case MOD_TELEFRAG:
						{
						// Set weapon name - mdavies
						pWeaponName = "Telefrag";
						break;
						}                            
					}
				}
			sl_LogScore( &gi, attacker->client->pers.netname, self->client->pers.netname,
						 "Kill", pWeaponName, 1, level.time );
			}
#endif*/
		}
	}

/************************************************************************
/*	Check if player should go straight into observer mode when entering
/*  game and ensure a supported model
/***********************************************************************/
void Chicken_CheckInPlayer(edict_t *ent)
	{
	// Set up hud stuff for player
	ent->client->ps.stats[STAT_CHICKEN_TIME_SHOW]	= CHICKEN_TIMER_OFF;
//	ent->client->ps.stats[STAT_CHICKEN_CLOCK]	= gi.imageindex (ICON_CLOCK);
//	ent->client->ps.stats[STAT_CHICKEN_COLON]	= gi.imageindex (ICON_CLOCK_COLON);

	// Setup player so as not to be throwing chicken. Which they aint
	ent->client->isThrowing	= false;

//	Chicken_CheckPlayerModel(ent); // Chicken

/*	if (chickenGame && teams)
		{
		if (ent->client->team != -1 && ent->client->modelOk)
			{
			ent->svflags		&= ~SVF_NOCLIENT;
			ent->client->ps.stats[STAT_CHICKEN_OBSERVER] = 1;
			}
		else
			Chicken_Observer(ent);
		}
	else
		{
		ent->svflags		&= ~SVF_NOCLIENT;
		ent->client->ps.stats[STAT_CHICKEN_OBSERVER] = 1;
		}*/
	}


//	Chicken Clock Routines
/************************************************************************
/*	Start a timer since someone musta picked up the chicken
/***********************************************************************/
static void Chicken_ClockStart (edict_t *ent)
	{
	if (ent)
		{
		edict_t *timer = G_Spawn();

		if (timer)
			{
			timer->classname	= "chicken_timer";
			timer->activator	= ent;
			timer->enemy		= ent;
			timer->health		= 0;
			timer->wait			= 0;
			timer->inuse		= true;
			timer->think		= Chicken_ClockThink;
			timer->nextthink	= level.time + 1;

			gi.linkentity(timer);
			}
		}
	else
		gi.dprintf("Chicken_ClockStart NULL");
	}

/************************************************************************
/*	Timer think routine used to update timer on screen and add score
/***********************************************************************/
static void Chicken_ClockThink (edict_t *timer)
	{
	qboolean	killTimer = false;
	gclient_t	*client;
	edict_t		*chickenholder;
	int			total, i;
	edict_t		*cl_ent;

	// Added the level.intermission so the timer stops at end of a game.
	if (timer && timer->enemy && timer->enemy->client && !level.intermissiontime)
		{
		client = timer->enemy->client;
		chickenholder = timer->enemy;

		if (client->pers.inventory[chickenItemIndex] == 0 || timer->enemy->deadflag == DEAD_DEAD)
			killTimer = true;
		else
			{
			timer->health++;
			timer->nextthink = level.time + 1;

			// Update the clock
			client->ps.stats[STAT_CHICKEN_TIME_MIN]  =  (timer->health - 1)/60;
			client->ps.stats[STAT_CHICKEN_TIME_1SEC] = ((timer->health - 1)/10)%6;
			client->ps.stats[STAT_CHICKEN_TIME_2SEC] =  (timer->health - 1)%10;

			// Should we play a sound?
			if (maxHoldScore > 0)
				{	
				if (strlen(SOUND_TIME_UP) && (scorePeriod*maxHoldScore) == (timer->health-1))
					{
					gi.sound(timer->enemy, CHAN_ITEM, gi.soundindex(SOUND_TIME_UP),   1, ATTN_STATIC, 0);
					DisplayHeldOverMessage(chickenholder);
					}
				else if (strlen(SOUND_TIME_LOW) && ((scorePeriod*maxHoldScore) - TIME_LOW_TIMEOUT + 1) <= timer->health)
					gi.sound(timer->enemy, CHAN_ITEM, gi.soundindex(SOUND_TIME_LOW),  1, ATTN_STATIC, 0);
				}

			// Has player scored yet..
			if (((timer->health-1)%scorePeriod) == 0 && timer->health > 1)
				{
				qboolean score = true;

					if (client->holdScore <= maxHoldScore || maxHoldScore == 0)
						{
						client->holdScore++;

						// Count the number of players in the game for scoring
						total = 0;
						for (i=0 ; i<game.maxclients ; i++)
							{
							cl_ent = g_edicts + 1 + i;

							if (!cl_ent->inuse)
								continue;

							total++;
							}

						// We can only score when more than 1 person is in the game.
						if (total > 1)
							client->resp.score++;

						// If we cant score anymore kill the timer and drop the chicken.
						if (client->holdScore == maxHoldScore && maxHoldScore > 0)
							{
							killTimer = true;
							Chicken_TossCheck(chickenholder);
							Chicken_Spawn();
							}
						}
					else
						score = false;
#ifdef STD_LOGGING
				if (score)
					sl_LogScore( &gi, client->pers.netname, NULL, "Chicken Held", NULL, 1,level.time );
#endif
				}
			gi.linkentity(timer);
			}
		}
	else
		killTimer = true;

	if (killTimer)
		{
		// Fail safe. Somethings wrong so kill the timer
		timer->nextthink		= 0;
		timer->health			= 0;
		G_FreeEdict (timer);
		}
	}

/************************************************************************
/*	Display chicken timer if player has chicken
/***********************************************************************/
void Chicken_Stats(edict_t *ent)
	{

	// Display Clock if player has chicken
	if (ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		ent->client->ps.stats[STAT_CHICKEN_TIME_SHOW] = CHICKEN_TIMER_ON;
	else
		{
		ent->client->ps.stats[STAT_CHICKEN_TIME_SHOW] = CHICKEN_TIMER_OFF;
		ent->client->ps.stats[STAT_CHICKEN_TIME_MIN]  = 0;
		ent->client->ps.stats[STAT_CHICKEN_TIME_1SEC] = 0;
		ent->client->ps.stats[STAT_CHICKEN_TIME_2SEC] = 0;
		}
	}

/************************************************************************
//	Select chicken spawn spot
/***********************************************************************/
static edict_t *Chicken_FindSpawnSpot(edict_t *ent)
	{
	edict_t		*spot = NULL;

	if (randomSpawn)
		spot = SelectRandomDeathmatchSpawnPoint (ent);
	else
		spot = SelectFarthestDeathmatchSpawnPoint (ent, false);

	if (!spot)
		{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
			{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
			}

		if (!spot && !game.spawnpoint[0] && (spot = G_Find(spot, FOFS(classname), "info_player_start")) == NULL)
			gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}

	return (spot);
	}

/************************************************************************
/*	Called when player wants to drop chicken from inventory
/***********************************************************************/
void Chicken_Drop(edict_t *ent,struct gitem_s *item)
	{
	// Dont want to crash is something passes crap
	if (ent && item)
		{
		// If players can drop and enough time has expired to allow a drop then do so
		if (!ent->client->pers.connected || (canDrop && ent->client->chickenTimer <= level.time))
			Chicken_Toss(ent,item);
		else
			{
			if (ent->client->nextInformTime < level.time)
				{
				if (canDrop)
					{
//					gi.cprintf (ent, PRINT_HIGH, "Can't drop %d second%s left\n", (int) (ent->client->chickenTimer - level.time + 1), ((int)(ent->client->chickenTimer - level.time + 1)) == 1?"":"s");
					}
				else
					gi.cprintf (ent, PRINT_HIGH, "You can't drop the chicken\n");

				ent->client->nextInformTime = level.time + 4.0;
				}
			}
		}
	else
		gi.dprintf("Chicken_Drop NULL");
	}

/************************************************************************
//	Force chicken to respawn regardless of where it is
//**********************************************************************/
int Chicken_Respawn(void *data, int itemId)
	{
	edict_t *ent = (edict_t *)data;

	if (chickenGame)
		{
		if (ent == NULL || respawnTime	< level.time)
			{
			Chicken_Spawn ();										
			if (ent && ent->client)
				gi.bprintf(PRINT_HIGH, "%s respawned Chicken\n", ent->client->pers.netname); 
			else if (itemId == 0)
				gi.bprintf(PRINT_HIGH, "Chicken respawned by server\n");
			else if (itemId == 1)
				gi.bprintf(PRINT_HIGH, "Chicken respawned by vote\n");

			if (ent && strlen(SOUND_CHICKEN_RESPAWN))
				gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_RESPAWN),  1, ATTN_NORM, 0);	
			}
		else
			gi.cprintf(ent, PRINT_HIGH, "Chicken cannot be respawned yet\n"); 
		}																
	else															
		gi.bprintf(PRINT_HIGH, "Catch the chicken not started\n");	

	return 0;
	}

/************************************************************************
/*	Chicken gun animation handling
/***********************************************************************/

void Chicken_Weapon (edict_t *ent)
	{
	static int	pause_frames[]	 = {32, 0};
	static int	fire_frames[]	 = {15, 0};
	static int  count			 = 1;
	static int  nextThrowFeather = 0;

	if (ent && ent->client && ent->client->bIsCamera != 1)
		{
		if (count >= nextThrowFeather)
			{
			// Throw random feathers
			Chicken_ThrowFeather (ent, 1);
			nextThrowFeather = feathers;
			
			count = 0;
			}
		count++;

		// If not in teamplay use generic weapon handling functions 
		if (teams == 0 && !canDrop)
			{
			if (!((int)(dmflags->value) & DF_MODELTEAMS))
				{
				Weapon_Generic (ent, 14, 31, 60, 61, pause_frames, fire_frames, Chicken_Fire);

				// Play random sound so people can locate player 
				Chicken_RandomSound(ent);
				}
			}
		else
			{
			if (ent->client->weaponstate == WEAPON_ACTIVATING)
				{
				// Show the reading chicken animation up till its ready
				if (ent->client->ps.gunframe == 14)
					{
					ent->client->weaponstate = WEAPON_READY;
					ent->client->ps.gunframe = 32;
					}
				else
					ent->client->ps.gunframe++;

				return;
				}

			// Chicken ready to throw
			if (ent->client->weaponstate == WEAPON_READY)
				{
				// Play random sound so people can locate player 
				Chicken_RandomSound(ent);

				// Is the fire button down
				if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
					{
					if (!ent->client->pers.connected || (canDrop && ent->client->chickenTimer <= level.time))
						{
						ent->client->latched_buttons	&= ~BUTTON_ATTACK;
						ent->client->ps.gunframe		= 62;
						ent->client->weaponstate		= WEAPON_FIRING;
						ent->client->grenade_time		= 0;
						return;
						}
					else
						{
						if (ent->client->nextInformTime < level.time)
							{
							if (canDrop)
								gi.cprintf (ent, PRINT_HIGH, "Can't throw %d second%s left\n", (int) (ent->client->chickenTimer - level.time + 1), ((int)(ent->client->chickenTimer - level.time + 1)) == 1?"":"s");
							else
								gi.cprintf (ent, PRINT_HIGH, "You can't throw the chicken\n");

							ent->client->nextInformTime = level.time + 4.0;
							}
						}
					}

				// Place pause every now and then
				if (ent->client->ps.gunframe == 32 && rand()&15)
					return;

				if (++ent->client->ps.gunframe > 60)
					ent->client->ps.gunframe = 32;
				return;
				}
	
			if (ent->client->weaponstate == WEAPON_FIRING)
				{
				if (ent->client->ps.gunframe == 63)
					{
					if (!ent->client->grenade_time)
						{
						// Generate a time for when chicken gets angry
						ent->client->grenade_time = level.time + CHICKEN_TIMER + 0.2;
						ent->client->weapon_sound = 0;

						if (strlen(SOUND_READY_TO_THROW))
							gi.sound(ent, CHAN_WEAPON, gi.soundindex(SOUND_READY_TO_THROW),  1, ATTN_NORM, 0);	
						}

					// they waited too long, let chicken peck em to death
					if (level.time >= ent->client->grenade_time)
						{
						if (strlen(SOUND_CHICKEN_ANGRY))
							gi.sound(ent, CHAN_WEAPON, gi.soundindex(SOUND_CHICKEN_ANGRY),  1, ATTN_NORM, 0);	

						T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 2, 0, DAMAGE_NO_KNOCKBACK, MOD_CHICKEN);

						}

					// Keep adding momentum if fire is held
					if (ent->client->buttons & BUTTON_ATTACK)
						return;

					// We're here if player has started to throw chicken

					// Change chicken item gun model to show hand throwing chicken
					chickenItem->view_model  = MODEL_WEAPON_THROW;
					ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
					ent->client->ps.gunframe = -1;
					}

				// Last throw frame
				if (ent->client->ps.gunframe == 4)
					{
					// Remove from players inventory once thrown
					Chicken_RemoveFromInventory();
					return;
					}

				if (ent->client->ps.gunframe == 1)
					Chicken_Fire(ent);

				ent->client->ps.gunframe++;
				}
			}
		}
	}

/************************************************************************
/*	Shoot chicken at something
/***********************************************************************/
static void Chicken_Fire(edict_t *ent)
	{
	if (ent && ent->client)
		{
		// If not in teamplay just make chicken do dumb stuff
		if (teams == 0 && !canDrop)
			{
			if (!((int)(dmflags->value) & DF_MODELTEAMS))
				{
				if (ent->client->ps.gunframe == 15 && strlen(SOUND_FIRE_WEAPON))
					gi.sound(ent, CHAN_AUTO, gi.soundindex(SOUND_FIRE_WEAPON),  1, ATTN_NORM, 0);	

				ent->client->weapon_sound = 0;
				ent->client->ps.gunframe++;
				}
			}
		else
			{
			vec3_t	offset;
			vec3_t	forward, right;
			vec3_t	start, up;
			float	timer;
			int		speed;
			edict_t	*chicken;

			timer = ent->client->grenade_time - level.time;
			speed = CHICKEN_MINSPEED + (CHICKEN_TIMER - timer) * ((CHICKEN_MAXSPEED - CHICKEN_MINSPEED) / CHICKEN_TIMER);

			if ((chicken = G_Spawn()) != NULL)
				{
				Chicken_Setup(chicken, ent);

				gi.setmodel (chicken, MODEL_CHICKEN_FLY);

				VectorSet		(offset, 8, 8, ent->viewheight-8);
				AngleVectors	(ent->client->v_angle, forward, right, up);
				P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

				VectorCopy  (start, chicken->s.origin);

				VectorScale (forward, speed, chicken->velocity);
				VectorMA    (chicken->velocity, 200 + crandom() * 10.0, up, chicken->velocity);
				VectorMA    (chicken->velocity, crandom() * 10.0, right, chicken->velocity);

				chicken->nextthink		= level.time + FRAMETIME;
				chicken->think			= Chicken_FlyThink; 

				VectorCopy (ent->s.angles, chicken->s.angles);

				gi.linkentity (chicken);

				ent->client->isThrowing = true;
				ent->client->ps.stats[STAT_CHICKEN_TIME_SHOW] = CHICKEN_TIMER_OFF;
				}
			}
		}
	else
		gi.dprintf("Chicken_Fire NULL");
	}

/************************************************************************
/*	Flying Chicken thinking routine called for throw chicken
/***********************************************************************/
static void Chicken_FlyThink (edict_t *self)
	{
	if (self)
		{
		if (self->groundentity)
			{
			// Reset model back to standing on ground
			gi.setmodel (self, MODEL_CHICKEN_ITEM);

			self->s.frame     = 0;
			self->s.angles[0] = 0;

			// Go into normal think rountine
			self->think = Chicken_Think; 
			}

		self->nextthink		= level.time + FRAMETIME;
		}
	else
		gi.dprintf("Chicken_FlyThink NULL");
	}

/************************************************************************
/*	Chucks a specified number of feathers out from player with chicken
/***********************************************************************/
static void Chicken_ThrowFeather (edict_t *self, int count)
	{
	// Loop around creating count number of feathers
	while (count > 0)
		{
		edict_t *feather;
		vec3_t	origin, size;

		if ((feather = G_Spawn()) != NULL)
			{
			feather->classname	= "feather";
			VectorScale (self->size,	0.5,	size);
			VectorAdd   (self->absmin,	size,	origin);

			feather->s.origin[0] = origin[0] + crandom() * size[0];
			feather->s.origin[1] = origin[1] + crandom() * size[1];
			feather->s.origin[2] = origin[2] + crandom() * size[2];

			gi.setmodel (feather, MODEL_FEATHER);

			feather->solid			= SOLID_NOT;
			feather->s.effects		= 0;
			feather->s.renderfx2	|= RF2_NOSHADOW;

			feather->flags			|= FL_NO_KNOCKBACK;
			feather->takedamage		= DAMAGE_NO;
			feather->die			= NULL;

			feather->movetype		= MOVETYPE_FLY;
			feather->touch			= NULL;
			feather->gravity		= 0.1;

			feather->velocity[0]	= 6.0 * crandom();
			feather->velocity[1]	= 6.0 * crandom();
			feather->velocity[2]	= -6;

			feather->avelocity[0]	= random()*300;
			feather->avelocity[1]	= random()*300;
			feather->avelocity[2]	= random()*300;

			feather->think			= G_FreeEdict;
			feather->nextthink		= level.time + 2 + random()*8;

			gi.linkentity (feather);

			count--;
			}
		else
			count = 0; // Get outta here no more space
		}
	}

/***************************************************************************
/*	Checks to see if a player can take damage while in CTC
/*	If true is returned quake will do its normal stuff otherwise no effect
//*************************************************************************/
int Chicken_TakeDamage(edict_t *self, edict_t *attacker, int damage)
	{
	// If player was attacked by the world. Let quake handle it
	if (attacker == world || (attacker && !attacker->client))
		return true;

	// Allow pain to dead player corpses
	if (self->client == NULL)
		return true;

	if (chickenGame)
		{
		vec3_t	vieworg;

		// If players got the chicken there fair game
		if (self && self->client && self->client->pers.inventory[chickenItemIndex] > 0)
			{
			char *painSound = NULL;

			if (damage < 25)						painSound = SOUND_CHICKEN_HURT_25;
			else if (damage >= 25 && damage < 50)	painSound = SOUND_CHICKEN_HURT_50;
			else if (damage >= 50 && damage < 75)	painSound = SOUND_CHICKEN_HURT_75;
			else									painSound = SOUND_CHICKEN_HURT_100;

			if (painSound && strlen(painSound))
				gi.sound(self, CHAN_ITEM, gi.soundindex(painSound),  1, ATTN_NORM, 0);	
			return true;
			}

		if (self && self->client)
			{
			VectorAdd (self->s.origin, self->client->ps.viewoffset, vieworg);

			// If in crap. Too bad you die
			if (gi.pointcontents (vieworg) & (CONTENTS_LAVA|CONTENTS_SLIME))
				return true;

			// If player was attacked by enemy team then ouch!!
//			if (teams != 0 && teamWithChicken == self->client->team)
//				return true;
			}
		}
	else
		return true;

	// Players safe.. I think
	return false;
	}

/************************************************************************
/*	Called when player dies and cant keep chicken
/***********************************************************************/

void Chicken_Toss(edict_t *ent,struct gitem_s *item)
{
	// Can never be too safe. Check for crap
	if (ent && item && ent->client && !ent->client->isThrowing)
		{
		edict_t *chicken = Drop_Item (ent, item);

		if (chicken) 
			Chicken_Setup (chicken, ent);

		// Remove chicken from players inventory
		Chicken_RemoveFromInventory();
		}
	}

/************************************************************************
/*
/*	Check to see if this player is a cheating bastard and it they are
/*	then let everyone know.
/***********************************************************************/
void CheckForCheating(edict_t* ent)
	{
	static qboolean timescalecheat = true;

	if (!ent->client)
		return;		// not fully in game yet

  	if (!ent->inuse)
    	return;

	// After 15 seconds recheck cheat, this is so we don't overflow.
	if ((ent->client->timescaletimer < level.time) && (timescalecheat))
		{
		// Retrieve the timescale
		gi.WriteByte(13);
		gi.WriteString("clienttimescale $timescale\n");
		gi.unicast(ent,true);

		// Reset time to next check for cheating.
		ent->client->timescaletimer = level.time + 15.0;

		// Check fixedtime next time
		timescalecheat = false;
		}
	else if((ent->client->timescaletimer < level.time) && (!timescalecheat))
		{
		// Retrieve the timescale
		gi.WriteByte(13);
		gi.WriteString("clientfixedtime $fixedtime\n");
		gi.unicast(ent,true);

		// Reset time to next check for cheating.
		ent->client->timescaletimer = level.time + 15.0;

		// Check timescale next time
		timescalecheat = true;
		}

	}

/************************************************************************
/*	Display a message to everyone that the chicken has been freed
/***********************************************************************/
void DisplayHeldOverMessage( edict_t *chookrustler )
	{
	edict_t *nextplayer = NULL;

	while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
		{
		if (nextplayer == chookrustler)
			gi.centerprintf(nextplayer, "%s, you're time is up.\nThe chicken is free.\n", chookrustler->client->pers.netname);
		else
			gi.centerprintf(nextplayer, "%s has lost the chicken!\nGo find it.\n", chookrustler->client->pers.netname);
		}

	}

/************************************************************************
/*
/*	Load the text file containing the user definable part of the MOTD
/***********************************************************************/
void LoadMOTD( void )
	{

	FILE	*motd_file;
	char	line[80];
	int		i;

	// Open the motd file
	if (motd_file = fopen("ctc/motd.txt", "r"))
		{
		i = 0;

		// Read the lines now
		while ( fgets(line, 80, motd_file) )
			{
			// Once we've read a line copy it to the MOTD array.
			strcpy(MOTD[i].textline, line);
			i++;

			// We don't want more than 3 lines so lets piss off.
			if (i>3)
				break;
			}

		// be good now ! ... close the file
		fclose(motd_file);
		}
	}


/***********************************************************************
/*
/*	Function:	Loads all the game settings.
/*
/*	Parameters:	None
/*
/**********************************************************************/
void LoadIniFile( void )
	{	
	FILE	*f;
	cvar_t	*game_dir;
	int		IniOption = 0, Processed = 0;
	char	Buffer[256], filename[256];
	char	*VariableName = NULL, *VariableValue = NULL;
	static	qboolean	AlreadyRead = false;


	if (AlreadyRead)
		return;

	game_dir = gi.cvar ("game", "", 0);

    sprintf(filename, ".\\%s\\%s", game_dir->string, INI_FILE);

	// open the *.ini file

	if ((f = fopen (filename, "r")) == NULL)
		{
		gi.dprintf("Unable to read %s. Using defaults.\n", INI_FILE);
		return;
		}

	gi.dprintf("\nProcessing CTC %s \n", INI_FILE);

	// read 256 characters or until we get to the eof or a return for a newline.

	while (fgets(Buffer, sizeof(Buffer), f) != NULL)
		{

		
		// Ignore this line if it starts with a #, newline, space or [ bracket.

		if (Buffer[0] != '\t' && Buffer[0] != ' ' && Buffer[0] != '\n' && Buffer[0] != '#' && Buffer[0] != '[')
			{

			// Get the variable name, skipping spaces, tabs, and newlines.

			VariableName	= strtok(Buffer, " \t\n");
			IniOption	= 0;

			// If we haven't processed the maximum number of options then keep going
			while (IniOption < MAX_OPTIONS)
				{

				// Find this option in the array of options, if we don't find it tough

				if (!strcmp(VariableName, option[IniOption].ident))
					{

					// Using NULL will continue the search for the value from where the previous
					// strtok for the variable name left off.
					VariableValue = strtok(NULL, " \t\n#");

					// If the variable name is stdlog then we want to set the flag to turn
					// logging on
					if (!strcmp(VariableName, "stdlog"))
						gi.cvar_set("stdlogfile", VariableValue);
					else
						// This will set the valu in the array using string value to integer conversion
						*option[IniOption].variable = atoi(VariableValue);

					Processed++;
					break;
					}

				IniOption++;
				}
			}
		}

	gi.dprintf("%d Options processed\n", Processed);
	fclose (f);
	AlreadyRead = true;	
	}

/***********************************************************************
/*
/*	Function:	Loads all the game settings.
/*
/*	Parameters:	None
/*
/**********************************************************************/
void VerifyIniFileValues( void )
	{	
	int	Loop;

	for ( Loop=0; Loop<MAX_OPTIONS; Loop++ )
		{

		// If the value which has been set isn't in the normal range then
		// set it to a default value.

		if ((*option[Loop].variable < option[Loop].MinVariable) ||
			(*option[Loop].variable > option[Loop].MaxVariable))
			{
			*option[Loop].variable = option[Loop].DefaultVariable;
			}
		}
	}

/***********************************************************************
/*
/*	Function:	Small function to return number of players in server. 
/*
/**********************************************************************/
static int PlayersInGame( void )
	{
	int			total,i;
	edict_t		*cl_ent;

	// Need to put this into a function to call.
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
		{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		total++;
		}

	return total;
	}

/***********************************************************************
/*
/*	Function:	Displays a superb message of the day 
/*
/**********************************************************************/
void MOTDScoreboardMessage (edict_t *ent)
	{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		yofs;
	char	*seperator = "++++++++++++++++++++++++++++++++++";

	char	*selectheader[] =
			{
			"Atrophy Presents",
			"-<> Catch The Chicken v1.1 <>-",
			"www.Atrophy.co.uk",
			"",
			"-: Credits :-",
			"Coding",
			"Rat Instinct",
			"Modellers",
			"[TCC]BadAss",
			"EvilBunny",
			"TiCaL",
			"Skinning",
			"MillyMog",
			"",
			"From the Q2 original by",
			"Jason Zuell, Simon & Paul Hulsinga",
			NULL
			};

	string[0] = 0;
	stringlength = 0;

	yofs = 15;

	// Atrophy, CTC, Presents
	for (i=0; i<3; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 090 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// Blank, Credits, Coding
	for (i=3; i<6; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 990 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// Rat Instinct
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 999 \"%s\" ",
		-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;
	i++;

	// Modellers
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 990 \"%s\" ",
		-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;
	i++;

	// Modellers names
	for (i=8; i<11; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// Skinning
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 990 \"%s\" ",
		-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;
	i++;

	// Millymog
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 999 \"%s\" ",
		-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;

	// Original Q2
	for (i=13; selectheader[i]; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 990 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// Server message.
	yofs += 5;
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 090 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;

	for (i=0; i< MOTD_lines; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 090 \"%s\" ",
			-5*strlen(MOTD[i].textline), yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		j = strlen(entry);
		if (stringlength + j < 1024)
			{
			strcpy (string + stringlength, entry);
			stringlength += j;
			}

		yofs += 20;
		}

	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 090 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);

	if (stringlength + j < 1024)
		{
		strcpy (string + stringlength, entry);
		stringlength += j;
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	}

/***********************************************************************
/*
/*	Function:	Displays what the current game config is.
/*
/**********************************************************************/
void ShowCTC_Config(edict_t *ent)
	{
	char	entry[1024];
	char	string[1400];
	char	*yesno;
	int		stringlength;
	int		i, j, IniOption;
	int		yofs, xpos;
	char	*seperator = "================================";

	char	*selectheader[] =
			{
			" CATCH THE CHICKEN CONFIGURATION ",
			"=================================",
			"Allow big health pickup       - ",
			"Allow small health pickup     - ",
			"Allow armour pickup           - ",
			"Score on killing chook holder - ",
			"Score on chook pickup         - ",
			"Stats logging                 - ",
			"Client chicken respawning     - ",
			"Auto chicken respawn          - ",
			"Auto chicken respawn delay    - ",
			"Score time                    - ",
			"Chicken delay before dropping - ",
			"Max score from holding chook  - ",
			"Player holding chicken glow   - ",
			NULL
			};

	yesno = NULL;
	
	string[0] = 0;
	stringlength = 0;

	yofs = 50;
	xpos = 90;

	for (i=0; i<2; i++)
		{
		Com_sprintf (entry, sizeof(entry),
			"xl %i yv %i dmstr 090 \"%s\" ", xpos , yofs + (int)(-60.0 + -3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	IniOption=0;

	// Main yes/no options
	for (i=2; i<10; i++)
		{
		if (*option[IniOption++].variable == 1)
			yesno = "Yes";
		else
			yesno = "No ";
			
		Com_sprintf (entry, sizeof(entry),
			"xl %i yv %i dmstr 999 \"%s%s\" ", xpos , yofs + (int)(-60.0 + -3.5*14), selectheader[i], yesno );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// Main seconds options
	for (i=10; selectheader[i]; i++)
		{
		
		Com_sprintf (entry, sizeof(entry),
		"xl %i yv %i dmstr 999 \"%s%i%s\" ",
		 xpos , yofs + (int)(-60.0 + -3.5*14), selectheader[i], *option[IniOption++].variable, "s" );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
		}

	// End of the table
	Com_sprintf (entry, sizeof(entry),
		"xl %i yv %i dmstr 090 \"%s\" ", xpos , yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	j = strlen(entry);

	if (stringlength + j < 1024)
		{
		strcpy (string + stringlength, entry);
		stringlength += j;
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	}
