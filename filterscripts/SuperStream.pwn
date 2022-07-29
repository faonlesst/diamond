#include <a_samp>

/*

Features prévues:

~~ - revoir logique de streaming..(sélectionner une partie des objets, faire quelques stream avec, puis quand sont outdated on change la sélection, vérifier déplacements via OnPlayerUpdate)
 - ajouter une variable qui pointe l'objet avec l'ID le plus haut

*/

//CONSTANT DECLARATION

#undef MAX_PLAYERS
#define MAX_PLAYERS 			100

//#define NO_TICK_COUNT                         //si jamais un problème est rencontré avec la fonction GetTickCount(), vous n'avez qu'a décommenter cette ligne et recompiler

//#define TIME_GRANULITY  		(50)            //précision du système de mesure du temps (permet de compenser GetTickCount() sous linux)
#define MOVEMENT_UPDATE 		(100)           //temps entre deux mises à jour de la positions des objets qui bougent
#define STREAMING_DELAY     	(750)           //temps minimal (en ms) depuis le dernier streaming pour exécuter le streaming via le timer
#define TIME_STREAMING      	(1000)          //cette valeur doit TOUJOURS être supérieur à celle au-dessus (STREAMING_DELAY), temps du timer pour le streaming

#define VIEWED_OBJECTS      	(175)

#define MAX_STREAM_OBJECTS  	(5000)

#define MAX_STREAM_DISTANCE     (350)       //distance à partir de laquelle on force un streaming (pour compenser le déplacement depuis le dernier streaming)

//CONSTANT USING FUNCTIONS DECLARATION

#define PointToPoint(%0,%1,%2,%3,%4,%5,%6)      (((%0 - %3) * (%0 - %3) + (%1 - %4) * (%1 - %4) + (%2 - %5) * (%2 - %5)) <= (%6 * %6))
#define GetSquaredDistance(%0,%1,%2,%3,%4,%5)   ((%0 - %3) * (%0 - %3) + (%1 - %4) * (%1 - %4) + (%2 - %5) * (%2 - %5))

#if defined NO_TICK_COUNT
	#define GetTickCount()                          (CurrentTick)
#endif

#define LastStreamPosUpdate(%0,%1,%2,%3,%4)		LastStreamPos[%0][0] = %1;\
												LastStreamPos[%0][1] = %2;\
												LastStreamPos[%0][2] = %3;\
												LastStreamPos[%0][3] = %4

#define IsValid(%0)                             (%0 < MAX_STREAM_OBJECTS && ObjectModel[%0])

#define MAJOR_VERSION                   1
#define MINOR_VERSION                   1
#define BUG_FIX                         4


//OBJECT RELATED VARIABLES

new ObjectModel[MAX_STREAM_OBJECTS];

new Float:ObjectPosX[MAX_STREAM_OBJECTS];
new Float:ObjectPosY[MAX_STREAM_OBJECTS];
new Float:ObjectPosZ[MAX_STREAM_OBJECTS];

new Float:ObjectPosRX[MAX_STREAM_OBJECTS];
new Float:ObjectPosRY[MAX_STREAM_OBJECTS];
new Float:ObjectPosRZ[MAX_STREAM_OBJECTS];

new Float:ObjectSpeed[MAX_STREAM_OBJECTS];
new Float:ObjectEndX[MAX_STREAM_OBJECTS];
new Float:ObjectEndY[MAX_STREAM_OBJECTS];
new Float:ObjectEndZ[MAX_STREAM_OBJECTS];

new CurrentMinID = 0;

//PLAYER RELATED VARIABLES

new Spawned[MAX_PLAYERS];
new LastStreamTime[MAX_PLAYERS];
new Float:LastStreamPos[MAX_PLAYERS][4];
new StreamedObjectsID[MAX_PLAYERS][MAX_STREAM_OBJECTS];

// STREAM RELATED VARIABLES

new MaximumID = 0;
new Float:ObjectDist[MAX_STREAM_OBJECTS/5];//on stock les distances ici
new ObjectID[MAX_STREAM_OBJECTS/5];//on stock les ID ici

//TIMER/CORE variables
new core_Timer;
new Move_Timer;
#if defined NO_TICK_COUNT
new CurrentTick = 0;
#endif


//FORWARDS

forward TimeUpdate();
forward MoveObjects();
forward core_Stream();
forward StreamPlayer(playerid, Float:PX, Float:PY, Float:PZ);
forward core_CreateObject(model, Float:OX, Float:OY, Float:OZ, Float:ORX, Float:ORY, Float:ORZ);
forward core_DestroyObject(objectid);
forward core_ClearPlayerObjects(playerid);
forward core_ClearAllObjects();
forward core_MoveObject(objectid, Float:TargetX, Float:TargetY, Float:TargetZ, Float:Speed);
forward core_StopObject(objectid);

public OnFilterScriptInit()
{
	print("|==========================================|");
	print("|   Loading SuperStream by Sim V" #MAJOR_VERSION "." #MINOR_VERSION "." #BUG_FIX "...   |");
	print("|==========================================|\n");
#if defined NO_TICK_COUNT
	SetTimer("TimeUpdate", TIME_GRANULITY, true);
#endif
	core_Timer = SetTimer("core_Stream", TIME_STREAMING, true);
	Move_Timer = SetTimer("MoveObjects", MOVEMENT_UPDATE, true);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
	    if(GetPlayerState(i) > PLAYER_STATE_NONE)
	    {
	        Spawned[i] = true;
	    }
	}
	print("|==========================================|");
	print("|        SuperStream V" #MAJOR_VERSION "." #MINOR_VERSION "." #BUG_FIX " loaded!        |");
	print("|==========================================|");
	return 1;
}

public OnFilterScriptExit()
{
   	print("|==========================================|");
	print("|  Unloading SuperStream by Sim V" #MAJOR_VERSION "." #MINOR_VERSION "." #BUG_FIX "...  |");
	print("|==========================================|\n");
	KillTimer(core_Timer);
	KillTimer(Move_Timer);
	for( new i = 0; i < MAX_PLAYERS; i++)
	{
		core_ClearPlayerObjects(i);
	}
	core_ClearAllObjects();
	print("|==========================================|");
	print("|       SuperStream V" #MAJOR_VERSION "." #MINOR_VERSION "." #BUG_FIX " unloaded!       |");
	print("|==========================================|");
	return 1;
}

public OnPlayerConnect(playerid)
{
	for(new i = 0; i < MAX_STREAM_OBJECTS; i++)
	{
	    StreamedObjectsID[playerid][i] = INVALID_OBJECT_ID;
	}
	LastStreamTime[playerid] = GetTickCount();
	Spawned[playerid] = false;
	return 1;
}

public OnPlayerDisconnect(playerid)
{
	core_ClearPlayerObjects(playerid);
	Spawned[playerid] = false;
	return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	Spawned[playerid] = true;
	return 1;
}

public OnPlayerSpawn(playerid)
{
	Spawned[playerid] = true;
	return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
	Spawned[playerid] = false;
	return 1;
}

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
    new Float:X, Float:Y, Float:Z;
    GetPlayerPos(playerid, X, Y, Z);
	core_ClearPlayerObjects(playerid);
    StreamPlayer(playerid, X, Y, Z);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	if(Spawned[playerid] && !IsPlayerNPC(playerid))
	{
	    if(GetTickCount() - LastStreamTime[playerid] > 600)
	    {
			new Float:X, Float:Y, Float:Z;
			GetPlayerPos(playerid, X, Y, Z);
			if(!PointToPoint(X, Y, Z, LastStreamPos[playerid][0], LastStreamPos[playerid][1], LastStreamPos[playerid][2], LastStreamPos[playerid][3]))
			{
			    StreamPlayer(playerid, X, Y, Z);
			}
		}
	}
	return 1;
}

#if defined NO_TICK_COUNT

public TimeUpdate()
{
	CurrentTick +=TIME_GRANULITY;
	return 1;
}

#endif

public MoveObjects()
{

	#define Fraction()	(MOVEMENT_UPDATE/1000)

	new i = 0;
	new Float:dx, Float:dy, Float:dz, Float:dp;
	for( i = 0; i < MAX_STREAM_OBJECTS; i++)
	{
	    if(ObjectSpeed[i])
	    {
	        dx = ObjectEndX[i] - ObjectPosX[i];
	        dy = ObjectEndY[i] - ObjectPosY[i];
	        dz = ObjectEndZ[i] - ObjectPosZ[i];
	        dp = floatsqroot(dx * dx + dy * dy + dz * dz);
	        if(dp <= ObjectSpeed[i] * Fraction())
	        {
	            ObjectPosX[i] += dx;
	            ObjectPosX[i] += dy;
	            ObjectPosX[i] += dz;
	            ObjectSpeed[i] = 0;
	            CallRemoteFunction("OnStreamObjectMoved", "i", i);
	        }
	        else
	        {
	        	ObjectPosX[i] += ObjectSpeed[i] * Fraction() * (dx/dp);
	        	ObjectPosY[i] += ObjectSpeed[i] * Fraction() * (dy/dp);
	        	ObjectPosZ[i] += ObjectSpeed[i] * Fraction() * (dz/dp);
			}
	    }
	}
	return 1;
}

public core_Stream()
{
	new Float:X, Float:Y, Float:Z;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
	    if(Spawned[i])
	    {
			if(GetPlayerPos(i, X, Y, Z))
			{
			    if(X == 0.0 && Y == 0.0 && Z == 0.0)
			    {
			    }
			    else if((GetTickCount() - LastStreamTime[i]) >= STREAMING_DELAY)
			    {
			        StreamPlayer(i, X, Y, Z);
			    }
			}
		}
	}
	return 1;
}

public StreamPlayer(playerid, Float:PX, Float:PY, Float:PZ)
{
	if(IsPlayerNPC(playerid))
	{
	    return 0;
	}
 	new object, i;
 	for(i = 0; i <= MaximumID; i++)
	{
	    if(ObjectModel[i])
	    {
     		ObjectDist[object] = GetSquaredDistance(PX, PY, PZ, ObjectPosX[i], ObjectPosY[i], ObjectPosZ[i]);
		    if(ObjectDist[object] <=  MAX_STREAM_DISTANCE * MAX_STREAM_DISTANCE)
		    {
		    	ObjectID[object] = i;
				object++;
			}
			else
			{
			    if(StreamedObjectsID[playerid][i] != INVALID_OBJECT_ID)
			    {
			        DestroyStreamedObject(playerid, i);
			    }
			}
		}
	}
	object--;
	QSort(ObjectDist, 0, object, ObjectID);//vérifier pour object-1

	while(object >= VIEWED_OBJECTS)//tant que les objets sont trop loin
	{
	    i = ObjectID[object];//on récupère l'objet
	    if(StreamedObjectsID[playerid][i] != INVALID_OBJECT_ID)//on teste s'il est montré
	    {
	        DestroyStreamedObject(playerid, i);//on détruit l'objet
	    }
	    object--;
	}
	new tmp = object;
	while(tmp >= 0)//tant que les objets sont trop loin
	{
	    i = ObjectID[tmp];
	    if(StreamedObjectsID[playerid][i] == INVALID_OBJECT_ID && ObjectModel[i])//on teste s'il est montré
	    {
	        CreateStreamedObject(playerid, i);//on crée l'objet
	    }
	    tmp--;
	}
	LastStreamTime[playerid] = GetTickCount();
	LastStreamPosUpdate(playerid, PX, PY, PZ, floatsqroot(ObjectDist[tmp])/1.5);
	return 1;
}

stock QSort(Float:numbers[], left, right, var2[])
{
	new
		Float:var = numbers[left],
		pivot = var2[left],
		l_hold = left,
		r_hold = right;
	while (left < right)
	{
		while ((numbers[right] >= var) && (left < right)) right--;
		if (left != right)
		{
			numbers[left] = numbers[right];
			var2[left] = var2[right];
			left++;
		}
		while ((numbers[left] <= var) && (left < right)) left++;
		if (left != right)
		{
			numbers[right] = numbers[left];
			var2[right] = var2[left];
			right--;
		}
	}
	numbers[left] = var;
	var2[left] = pivot;
	pivot = left;
	if (l_hold < pivot) QSort(numbers, l_hold, pivot - 1, var2);
	if (r_hold > pivot) QSort(numbers, pivot + 1, r_hold, var2);
}

CreateStreamedObject(playerid, objectid)
{
	StreamedObjectsID[playerid][objectid] = CreatePlayerObject(playerid, ObjectModel[objectid], ObjectPosX[objectid], ObjectPosY[objectid], ObjectPosZ[objectid], ObjectPosRX[objectid], ObjectPosRY[objectid], ObjectPosRZ[objectid]);
	if(ObjectSpeed[objectid])
	{
		MovePlayerObject(playerid, StreamedObjectsID[playerid][objectid], ObjectEndX[objectid], ObjectEndY[objectid], ObjectEndZ[objectid], ObjectSpeed[objectid]);
 	}
	return 1;
}

DestroyStreamedObject(playerid, objectid)
{
	new objid = StreamedObjectsID[playerid][objectid];
	if(IsValidPlayerObject(playerid, objid))
	{
		DestroyPlayerObject(playerid, objid);
	}
	StreamedObjectsID[playerid][objectid] = INVALID_OBJECT_ID;
 	return 1;
}

//FONCTIONS ACCESSIBLES DEPUIS L'INCLUDE

public core_CreateObject(model, Float:OX, Float:OY, Float:OZ, Float:ORX, Float:ORY, Float:ORZ)//valeur de retour: ID de l'objet créé, -1 si invalide
{
	if(CurrentMinID >= MAX_STREAM_OBJECTS)
	{
		printf("Can't create object (%d, %f, %f, %f, %f, %f, %f), limit reached!", model, OX, OY, OZ, ORX, ORY, ORZ);
	    return -1;//ne peut créer l'objet, plein
	}
	if(model <= 0)
	{
		printf("Can't create object (%d, %f, %f, %f, %f, %f, %f), model invalid!", model, OX, OY, OZ, ORX, ORY, ORZ);
	    return -1;//ne peut créer l'objet, plein
	}
	while(ObjectModel[CurrentMinID])
	{
	    CurrentMinID++;
	    if(CurrentMinID >= MAX_STREAM_OBJECTS)
	    {
			printf("Can't create object (%d, %f, %f, %f, %f, %f, %f), limit reached!", model, OX, OY, OZ, ORX, ORY, ORZ);
		    return -1;//ne peut créer l'objet, plein
	    }
	}
	if(MaximumID < CurrentMinID)
	{
	    MaximumID = CurrentMinID;
	}
	ObjectModel[CurrentMinID] = model;
	ObjectPosX[CurrentMinID] = OX;
	ObjectPosY[CurrentMinID] = OY;
	ObjectPosZ[CurrentMinID] = OZ;
	ObjectPosRX[CurrentMinID] = ORX;
	ObjectPosRY[CurrentMinID] = ORY;
	ObjectPosRZ[CurrentMinID] = ORZ;
	ObjectSpeed[CurrentMinID] = 0;
	return CurrentMinID + 1;//retour de l'ID(s'assure que les id des objets retournées != 0)
}

public core_DestroyObject(objectid)//valeur de retour: ID de l'objet créé, -1 si invalide
{
	objectid -= 1;//s'assure que les id des objets retournées != 0
	if(objectid >= MAX_STREAM_OBJECTS || objectid < 0)
	{
		//printf("Can't delete object %d, array out of bounds!", objectid + 1);
	    return 0;//ne peut créer l'objet, plein
	}
	if(ObjectModel[objectid])
	{
		ObjectModel[objectid] = 0;
		ObjectPosX[objectid] = 0;
		ObjectPosY[objectid] = 0;
		ObjectPosZ[objectid] = 0;
		ObjectPosRX[objectid] = 0;
		ObjectPosRY[objectid] = 0;
		ObjectPosRZ[objectid] = 0;
		ObjectSpeed[objectid] = 0;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
		    DestroyStreamedObject(i, objectid);
		}
	}
	else
	{
		printf("Can't delete object %d, it doesn't exist!", objectid + 1);
		return 0;
	}
	if(CurrentMinID > objectid)
	{
	    CurrentMinID = objectid;
	}
	if(MaximumID == objectid)
	{
	    while(ObjectModel[MaximumID] == 0)
	    {
	        MaximumID--;
	    }
	}
	return 1;//retour de l'ID
}

public core_ClearPlayerObjects(playerid)
{
	for( new i = 0; i < MAX_STREAM_OBJECTS; i++)
	{
		if(StreamedObjectsID[playerid][i])
		{
		    DestroyStreamedObject(playerid, i);
		}
	}
	return 1;
}

public core_ClearAllObjects()
{
	for( new i = 1; i <= MAX_STREAM_OBJECTS; i++)
	{
	    if(ObjectModel[i])
	    {
			core_DestroyObject(i);
		}
	}
	return 1;
}



public core_MoveObject(objectid, Float:TargetX, Float:TargetY, Float:TargetZ, Float:Speed)//valeur de retour: 0 si objet invalide, 1 si objet en mouvement
{
	new id = 0;
	objectid -= 1;//s'assure que les id des objets retournées != 0
	if(IsValid(objectid))
	{
	    ObjectEndX[objectid] = TargetX;
	    ObjectEndY[objectid] = TargetY;
	    ObjectEndZ[objectid] = TargetZ;

 	    ObjectSpeed[objectid] = Speed;

 	    for(new i = 0; i < MAX_PLAYERS; i++)
 	    {
 	        if((id = StreamedObjectsID[i][objectid]))
 	        {
 	            MovePlayerObject(i, id, TargetX, TargetY, TargetZ, Speed);
 	        }
 	    }
 	    id = 1;
	}
	return id;
}

public core_StopObject(objectid)//valeur de retour: 1 si réussit, 0 si impossible
{
	new id = 0;
	objectid -= 1;//s'assure que les id des objets retournées != 0
	if(IsValid(objectid))
	{
	    if(ObjectSpeed[objectid])
	    {
	        //on s'assure de stopper les calculs par la fonction MoveObjects
	        ObjectSpeed[objectid] = 0;

	        //On stop l'objet pour tous les joueurs
	 	    for(new i = 0; i < MAX_PLAYERS; i++)
	 	    {
	 	        if((id = StreamedObjectsID[i][objectid]))
	 	        {
	 	            StopPlayerObject(i, id);
	 	        }
	 	    }
	 	    id = 1;
	    }
	}
	return id;
}
