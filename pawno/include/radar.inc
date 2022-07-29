#include <a_samp>

/*
native CreateRadar(Float:X,Float:Y,Float:Z,Float:A,vitesse = DEFAULT_VITESSE,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE);
native DestroyRadar(radarid);
native GetRadarPos(radarid,&Float:X,&Float:Y,&Float:Z,&Float:A);
native SetRadarPos(radarid,Float:X,Float:Y,Float:Z,Float:A,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE);
native SetRadarVitesse(radarid,vitesse);
native GetRadarVitesse(radarid);
native AttachRadarToPlayer(playerid,radarid)
native DettachRadarToPlayer(playerid,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE);
native IsValidRadar(radarid);
native IsRadarAttach(radarid);
native IsPlayerRadarAttach(playerid);
native GetPlayerRadarAttach(playerid);
native SetZoneRadar(radarid,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE);
native GetRadarProxPlayer(playerid,Float:radi = DEFAULT_PROX_RADAR);
native GetRadarObjectID(radarid);
native GetVehicleSpeed(vehicleid);
native GetNbRadar();
*/

#define MAX_RADAR 20 // Nombre maximum de radar
#define DEFAULT_DIST_CENTRE_ZONE 7.0 //Distance par defaut du centre d'une zone de flash d'un radar par rapport au radar
#define DEFAULT_VITESSE 100 //Vitesse de flash par defaut (pas vraiment utilie, car la vitesse de flash d'un radar peux être defini via SetRadarVitesse ;))
#define DEFAULT_PROX_RADAR 2.0 //Zone de detection de radar par defaut de la fonction GetRadarProxPlayer

#define GETVEHICLESPEED GetVehicleSpeed /*Nom de la fonction qui recupere la vitesse d'un vehicule (voir explication ci dessou)

	- Si vous avez deja votre propre fonction pour connaitre la vitesse d'un vehicule suprimer la fonction 'GetVehicleSpeed' et remplacer la valeur de la define GETVEHICLESPEED par le nom de votre fonction

	!Votre fonction de calcul de la vitesse doit s'attendre a recevoir comme parametre l'id d'un vehicule et renvoyer un entier.
		
	- Si votre fonction ne respecte pas les condition si dessus:

		modifier la ligne:

			if((vitesse = GETVEHICLESPEED(GetPlayerVehicleID(playerid))) > RadarInfo[r][Vitesse])
		
	    par:

		if((vitesse = VOTREFONCTION(VOSPARAMETRE)) > RadarInfo[r][Vitesse])
	    
	!n'oublier pas de remplacer VOTREFONCTION par le nom de votre fonction et VOSPARAMETRE par les parametre requis pour votre fonction (si votre fonction attent l'id d'un joueur metter 'playerid')

 	- Si votre fonction de calcul de la vitesse renvoi une valeur de type Float

		Modifier la ligne:

			vitesse = 0,
		
		par:
		
			Float:vitesse = 0.0,

*/





#define INVALID_RADAR_ID -1
#define VERSION "v1.0"

#define IsValidRadar(%0) (RadarInfo[%0][created])  //Verifie si un radar est valid (parametre qu'attent la fonction: id d'un radar)
#define IsRadarAttach(%0) ((RadarInfo[%0][pattach] != INVALID_PLAYER_ID)) //Verifie si un radar est attacher a un joueur (parametre qu'attent la fonction: id d'un radar)
#define GetPlayerRadarAttach(%0) (RadarInfo[%0][pattach])    //Renvoi l'id du joueur auquel un radar est attacher (parametre qu'attent la fonction: id d'un radar)
#define SetRadarVitesse(%0,%1) RadarInfo[%0][Vitesse] = (%1) //Defini la vitesse a partir de laquel le radar va flasher les joueur (parametre qu'attent la fonction: id d'un radar et vitesse (entier))
#define GetRadarVitesse(%0) (RadarInfo[%0][Vitesse]) //Renvoi la vitesse a partir de laquel le radar va flasher les joueur (paramtre qu'attent la fonction: id d'un radar)

#define IsPlayerRadarAttach(%0) ((PlayerRadarAttached[%0] != INVALID_RADAR_ID))//Verifie si un joueur a un radar attacher a lui (parametre qu'attent la fonction: id d'un joueur)
#define GetRadarPlayerAttach(%0) (PlayerRadarAttached[%0])//Renvoi l'id du radar attacher au joueur (parametre qu'attent la fonction: id d'un joueur)

forward HideFlash(playerid);
forward OnPlayerFlashed(playerid,radarid,vitesse);

/*========[VARIABLE GLOBALE]======*/
enum RADARINFO {

	bool:created = false,
	pattach,
	Vitesse = DEFAULT_VITESSE,
	ObjectID = INVALID_OBJECT_ID,
	Float:PosX,
	Float:PosY,
	Float:PosZ,
	Float:PosA,
	Float:ZoneMinX,
	Float:ZoneMinY,
	Float:ZoneMaxX,
	Float:ZoneMaxY,
	Float:ZoneMinZ,
	Float:ZoneMaxZ,
}
new RadarInfo[MAX_RADAR][RADARINFO];
static Text:FlashRadar;
static PlayerFlashed[MAX_PLAYERS] = INVALID_RADAR_ID;
new PlayerRadarAttached[MAX_PLAYERS] = INVALID_RADAR_ID;


/*========================================[FONCTION DU SYSTEME]=============================================*/


/*----------------------------------------------------------------------------*-
Function:
	CreateRadar
Params:
	Float:X				- Position X du radar
	Float:Y				- Position Y du radar
	Float:Z				- Position Z du radar
	Float:A				- Angle du radar
	vitesse     		- Vitesse a laquel le radar flashera les joueur
	Float:distcentre  	- Distance a laquel se situera le centre de la zone de detection du radar (defini aussi la taille de la zone de detection)
Return:
	int
Notes:
	Cree un radar et renvoi l'id du radar.
	Si le nombre de radar maximum a été atteint cette fonction renvera INVALID_RADAR_ID

-*----------------------------------------------------------------------------*/
stock CreateRadar(Float:X,Float:Y,Float:Z,Float:A,vitesse = DEFAULT_VITESSE,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE)
{
	for(new r=0;r<MAX_RADAR;r++)
	{
	    if(!IsValidRadar(r))
	    {
	        RadarInfo[r][created] = true;
	        RadarInfo[r][pattach] = INVALID_PLAYER_ID;
			RadarInfo[r][Vitesse] = vitesse;
			RadarInfo[r][PosX] = X;
			RadarInfo[r][PosY] = Y;
			RadarInfo[r][PosZ] = Z;
			RadarInfo[r][PosA] = A;
			RadarInfo[r][ObjectID] = CreateObject(1886,X,Y,Z,0.0,180.0,A);
			SetZoneRadar(r,distcentre);
            return r;
	    }
	}
	return INVALID_RADAR_ID;
}

/*----------------------------------------------------------------------------*-
Function:
	DestroyRadar
Params:
	radarid     - ID du radar
Return:
	bool
Notes:
	Detruit un radar

-*----------------------------------------------------------------------------*/
stock DestroyRadar(radarid)
{
	if(!IsValidRadar(radarid))
	    return false;

    DestroyObject(RadarInfo[radarid][ObjectID]);
    RadarInfo[radarid][created] = false;
    RadarInfo[radarid][pattach] = INVALID_PLAYER_ID;
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	GetRadarPos
Params:
	radarid     - ID du radar
	&Float:X    - Variable de reception de la la position X
    &Float:Y    - Variable de reception de la la position Y
    &Float:Z    - Variable de reception de la la position Z
    &Float:A    - Variable de reception de l'angle
Return:
	bool
Notes:
	Renvoi la position d'un radar (X,Y,Z,A)

-*----------------------------------------------------------------------------*/
stock GetRadarPos(radarid,&Float:X,&Float:Y,&Float:Z,&Float:A)
{
	if(!IsValidRadar(radarid))
	    return false;

	X = RadarInfo[radarid][PosX];
	Y = RadarInfo[radarid][PosY];
	Z = RadarInfo[radarid][PosZ];
	A = RadarInfo[radarid][PosA];
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	SetRadarPos
Params:
	radarid     		- ID du radar
	Float:X				- Position X du radar
	Float:Y				- Position Y du radar
	Float:Z				- Position Z du radar
	Float:A				- Angle du radar
	Float:distcentre  	- Distance a laquel se situera le centre de la zone de detection du radar (defini aussi la taille de la zone de detection)
Return:
	bool
Notes:
	Modifie la position du radar

-*----------------------------------------------------------------------------*/
stock SetRadarPos(radarid,Float:X,Float:Y,Float:Z,Float:A,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE)
{
	if(!IsValidRadar(radarid) || IsRadarAttach(radarid))
	    return false;

	RadarInfo[radarid][PosX] = X;
	RadarInfo[radarid][PosY] = Y;
	RadarInfo[radarid][PosZ] = Z;
	RadarInfo[radarid][PosA] = A;
	SetObjectPos(RadarInfo[radarid][ObjectID],X,Y,Z);
	SetZoneRadar(radarid,distcentre);
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	AttachRadarToPlayer
Params:
	radarid     - ID du radar
	playerid    - ID du joueur
Return:
	bool
Notes:
	Attache un radar a un joueur (permet de deplacer le radar par exemple)

-*----------------------------------------------------------------------------*/
stock AttachRadarToPlayer(radarid,playerid)
{
	if(!IsPlayerConnected(playerid) || !IsValidRadar(radarid) || IsRadarAttach(radarid) || IsPlayerRadarAttach(playerid))
	    return false;

	RadarInfo[radarid][pattach] = playerid;
	AttachObjectToPlayer(RadarInfo[radarid][ObjectID],playerid,0.0,1.0,-1.0,0.0,180.0,180.0);
	PlayerRadarAttached[playerid] = radarid;
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	DettachRadarToPlayer
Params:
	radarid     		- ID du radar
	Float:distcentre  	- Distance a laquel se situera le centre de la zone de detection du radar (defini aussi la taille de la zone de detection)
Return:
	bool
Notes:
	Dettache un radar d'un joueur
	Cette fonction replace directement le radar devant le joueur et recalcule la zone d'effet du radar

-*----------------------------------------------------------------------------*/
stock DettachRadarToPlayer(playerid,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE)
{
	if(!IsPlayerConnected(playerid) || !IsPlayerRadarAttach(playerid))
	    return false;
	new radarid = GetRadarPlayerAttach(playerid);
	if(!IsValidRadar(radarid))
		return false;
		
	DestroyObject(RadarInfo[radarid][ObjectID]);
	new Float:X,Float:Y,Float:Z,Float:A;
	GetPlayerPos(playerid,X,Y,Z);
	GetPlayerFacingAngle(playerid,A);
	GetXYFrontAngle(X,Y,A,1.0);
	RadarInfo[radarid][PosX] = X;
	RadarInfo[radarid][PosY] = Y;
	RadarInfo[radarid][PosZ] = Z;
	RadarInfo[radarid][PosA] = A;
	RadarInfo[radarid][ObjectID] = CreateObject(1886,X,Y,Z-1.0,0.0,180.0,A+180.0);
	SetZoneRadar(radarid,distcentre);
	RadarInfo[radarid][pattach] = INVALID_PLAYER_ID;
	PlayerRadarAttached[playerid] = INVALID_RADAR_ID;
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	SetZoneRadar
Params:
	radarid     		- ID du radar
	Float:distcentre  	- Distance a laquel se situera le centre de la zone de detection du radar (defini aussi la taille de la zone de detection)
Return:
	bool
Notes:
	Recalcule la zone d'effet du radar

-*----------------------------------------------------------------------------*/
stock SetZoneRadar(radarid,Float:distcentre = DEFAULT_DIST_CENTRE_ZONE)
{
	if(!IsValidRadar(radarid) && !IsRadarAttach(radarid))
	    return false;
	    
	new Float:X = RadarInfo[radarid][PosX],Float:Y = RadarInfo[radarid][PosY],Float:A = RadarInfo[radarid][PosA];
	new Float:coter = floatmul(distcentre,2); //Calcul d'un coté du caré (distance du centre par rapport au radar multiplié par 2)
	new Float:carercoter = floatmul(coter,coter);//Calcul du caré d'un coté (coter * coter)
	new Float:diagonale = floatsqroot((carercoter+carercoter)); // Calcul de la diagonale du caré (racine carrée de carercoter+carercoter)
	
	GetXYFrontAngle(X,Y,A,distcentre); // Calcul de la position X et Y du centre du caré situé en face du radar

	new Float:X1 = X,Float:Y1 = Y,Float:X2 = X,Float:Y2 = Y;
	GetXYFrontAngle(X1,Y1,A+45.0,(diagonale/2)); //Calcul de la position X,Y du coin 1 du caré
	GetXYFrontAngle(X2,Y2,A+225.0,(diagonale/2));//Calcul de la position X,Y du coin 2 du caré
	
	//Recherche du Xmin et Xmax
	if(X1<X2)
	{
		RadarInfo[radarid][ZoneMinX] = X1;
		RadarInfo[radarid][ZoneMaxX] = X2;
	}
	else
	{
		RadarInfo[radarid][ZoneMinX] = X2;
		RadarInfo[radarid][ZoneMaxX] = X1;
	}
	//Recherche du Ymin et Ymax
	if(Y1<Y2)
	{
		RadarInfo[radarid][ZoneMinY] = Y1;
		RadarInfo[radarid][ZoneMaxY] = Y2;
	}
	else
	{
		RadarInfo[radarid][ZoneMinY] = Y2;
		RadarInfo[radarid][ZoneMaxY] = Y1;
	}
	RadarInfo[radarid][ZoneMinZ] = RadarInfo[radarid][PosZ]-distcentre;
	RadarInfo[radarid][ZoneMaxZ] = RadarInfo[radarid][PosZ]+distcentre;
	return true;
}

/*----------------------------------------------------------------------------*-
Function:
	GetRadarProxPlayer
Params:
	playerid     	- ID du joueur
	Float:radi  	- rayon dans lequel rechercher un radar
Return:
	int
Notes:
	Cette fonction cherche le radar le plus proche du joueur et renvoi sont id .
	Si aucun radar n'est trouver dans le rayon defini cette fonction renvera INVALID_RADAR_ID

-*----------------------------------------------------------------------------*/
stock GetRadarProxPlayer(playerid,Float:radi = DEFAULT_PROX_RADAR)
{
	new Float:pos[3],Float:tmppos[3];
	if(IsPlayerConnected(playerid))
	{
	    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
		for(new r=0;r<MAX_RADAR;r++)
		{
		    if(IsValidRadar(r) && !IsRadarAttach(r))
		    {
				tmppos[0] = (pos[0] - RadarInfo[r][PosX]);
				tmppos[1] = (pos[1] - RadarInfo[r][PosY]);
				tmppos[2] = (pos[2] - RadarInfo[r][PosZ]);
				if (((tmppos[0] < radi) && (tmppos[0] > -radi)) && ((tmppos[1] < radi) && (tmppos[1] > -radi)) && ((tmppos[2] < radi) && (tmppos[2] > -radi)))
				{
					return r;
				}
			}
		}
	}
	return INVALID_RADAR_ID;
}

/*----------------------------------------------------------------------------*-
Function:
	GetRadarObjectID
Params:
	radarid     	- ID du radar
Return:
	int
Notes:
	Cette fonction renvoi l'id de l'objet associé a un radar, si le radar n'existe pas cette fonction renvoi false

-*----------------------------------------------------------------------------*/
stock GetRadarObjectID(radarid)
{
	if(!IsValidRadar(radarid))
	    return false;

	return RadarInfo[radarid][ObjectID];
}

/*----------------------------------------------------------------------------*-
Function:
	GetNbRadar
Params:
	    /
Return:
	int
Notes:
	Renvoi le nombre de radar actuellement posé

-*----------------------------------------------------------------------------*/
stock GetNbRadar()
{
	new count = 0;
	for(new r=0;r<MAX_RADAR;r++)
	{
	    if(!IsValidRadar(r))
	        continue;
		count++;
	}
	return count;
}

/*----------------------------------------------------------------------------*-
Function:
	GetVehicleSpeed
Params:
	vehicleid  - Id d'un vehicule
Return:
	int
Notes:
	Renvoi le vitesse (en km/h) d'un vehicule.
	
	/!\ Si vous avez deja votre propre fonction pour connaitre la vitesse d'un vehicule suprimer cette fonction et remplacer la valeur de la define GETVEHICLESPEED par le nom de votre fonction
	ps:
		votre fonction de calcul de la vitesse doit s'attendre a recevoir comme parametre l'id d'un vehicule sinon modifier la ligne:
	    'if((vitesse = GETVEHICLESPEED(GetPlayerVehicleID(i))) > RadarInfo[r][Vitesse])'
	    par:
	    'if((vitesse = VOTREFONCTION(VOSPARAMETRE)) > RadarInfo[r][Vitesse])'
	    
	    n'oublier pas de remplacer VOTREFONCTION par le nom de votre fonction et VOSPARAMETRE par les parametre requis pour votre fonction
	

-*----------------------------------------------------------------------------*/
stock GetVehicleSpeed(vehicleid)
{
    new Float:X, Float:Y, Float:Z;
    GetVehicleVelocity(vehicleid, X, Y, Z);
    return floatround(floatsqroot(X * X + Y * Y + Z * Z) * 180);
}




/*===========[COEUR DU SYSTEME]================*/

forward Float:GetXYFrontAngle(&Float:x, &Float:y, Float:a, Float:distance);
public Float:GetXYFrontAngle(&Float:x, &Float:y, Float:a, Float:distance)
{
	x += (distance * floatsin(-a, degrees));
	y += (distance * floatcos(-a, degrees));
	return Float:a;
}
#if defined FILTERSCRIPTS
public OnFilterScriptInit()
#else
public OnGameModeInit()
#endif
{
    for(new r=0;r<MAX_RADAR;r++)
    {
		RadarInfo[r][created] = false;
		RadarInfo[r][pattach] = (INVALID_PLAYER_ID);
		RadarInfo[r][Vitesse] = DEFAULT_VITESSE;
		RadarInfo[r][ObjectID] = INVALID_OBJECT_ID;
	}

	/*====[TEXT DRAW DU FLASH DU RADAR]====*/
	FlashRadar = TextDrawCreate(0.0,0.0, "-");
	TextDrawUseBox(FlashRadar,1);
	TextDrawColor(FlashRadar,0xFFFFFF99);
	TextDrawBoxColor(FlashRadar,0xFFFFFF99);
	TextDrawTextSize(FlashRadar,640,480);
	TextDrawLetterSize(FlashRadar,640,480);
	print("=========================================");
	print("  [INC] Radar System "#VERSION" By MrFredo");
	print("=========================================");
	return Radar_Init();
}
forward Radar_Init();
#if defined FILTERSCRIPTS
	#define OnFilterScriptInit Radar_Init
#else
	#define OnGameModeInit Radar_Init
#endif

#if defined FILTERSCRIPTS
public OnFilterScriptExit()
#else
public OnGameModeExit()
#endif
{
	for(new r=0;r<MAX_RADAR;r++)
	{
	    if(IsValidObject(RadarInfo[r][ObjectID]))
	        DestroyObject(RadarInfo[r][ObjectID]);

		RadarInfo[r][created] = false;
		RadarInfo[r][ObjectID] = INVALID_OBJECT_ID;
	}

	TextDrawDestroy(FlashRadar);

	return 1;
}


public OnPlayerConnect(playerid)
{
	PlayerRadarAttached[playerid] = INVALID_RADAR_ID;
	PlayerFlashed[playerid] = INVALID_RADAR_ID;
	return Radar_PlayerConnect(playerid);
}

forward Radar_PlayerConnect(playerid);
#define OnPlayerConnect Radar_PlayerConnect

public OnPlayerDisconnect(playerid,reason)
{
	if(IsPlayerRadarAttach(playerid))
   		DestroyRadar(PlayerRadarAttached[playerid]);

	PlayerRadarAttached[playerid] = INVALID_RADAR_ID;
	PlayerFlashed[playerid] = INVALID_RADAR_ID;
    return Radar_PlayerDisconnect(playerid,reason);
}

forward Radar_PlayerDisconnect(playerid,reason);
#define OnPlayerDisconnect Radar_PlayerDisconnect
public OnPlayerUpdate(playerid)
{
	if(PlayerFlashed[playerid] == INVALID_RADAR_ID)// Si il na pas été flasher
	{
		if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
	    	PlayerRadarSystem(playerid);
	}
	else//Si il a été flasher
	{
	    new radarid = PlayerFlashed[playerid];
	    new Float:X,Float:Y,Float:Z;
	    GetPlayerPos(playerid,X,Y,Z);
		if(!(X > RadarInfo[radarid][ZoneMinX] && X < RadarInfo[radarid][ZoneMaxX] && Y > RadarInfo[radarid][ZoneMinY] && Y < RadarInfo[radarid][ZoneMaxY]))//Si il n'est plus dans la zone du radar ou il c'est fait flasher
			PlayerFlashed[playerid] = INVALID_RADAR_ID;//On remet la variable a INVALID_RADAR_ID
	}
	return Radar_PlayerUpdate(playerid);
}

forward Radar_PlayerUpdate(playerid);
#define OnPlayerUpdate Radar_PlayerUpdate

PlayerRadarSystem(playerid)
{
	new
		vitesse = 0,
		Float:X,Float:Y,Float:Z;
	GetPlayerPos(playerid,X,Y,Z);//Recuperation de la position du joueur
 	for(new r=0;r<MAX_RADAR;r++)//Boucle sur tout les radar
  	{
   		if(IsValidRadar(r) && !IsRadarAttach(r)) //On verifie que le radar est valide et qu'il n'est pas attacher a quelqu'un
     	{
			if(X > RadarInfo[r][ZoneMinX] && X < RadarInfo[r][ZoneMaxX] && Y > RadarInfo[r][ZoneMinY] && Y < RadarInfo[r][ZoneMaxY] && Z > RadarInfo[r][ZoneMinZ] && Z < RadarInfo[r][ZoneMaxZ]) //On verifie que le joueur est dans la zone d'effet du radar
			{
		    	if((vitesse = GETVEHICLESPEED(GetPlayerVehicleID(playerid))) > RadarInfo[r][Vitesse]) // si la vitesse du vehicule du joueur est superieur a c'elle autoriser par le radar
	    		{
		    	    PlayerFlashed[playerid] = r;
					Core_OnPlayerFlashed(playerid,r,vitesse);//On flash le joueur
					return 1;
				}
			}
		}
	}
	return 1;
}
Core_OnPlayerFlashed(playerid,radarid,vitesse)
{
	if(OnPlayerFlashed(playerid,radarid,vitesse))//Si notre callback OnPlayerFlashed renvoi true on execute le flash de l'ecran
	{
		TextDrawShowForPlayer(playerid,FlashRadar); // On afiche le textdraw blanc
		SetTimerEx("HideFlash",80,0,"i",playerid);// durant 80 Ms se qui a pour effet de donné une impression de flash de l'ecran :D
		PlayerPlaySound(playerid,1132,0.0,0.0,0.0); //On ajoute a sa le bruit d'appareil photo pour que le joueur se rende bien compte du flash :D
	}
}

public HideFlash(playerid)
{
	TextDrawHideForPlayer(playerid,FlashRadar);
}
