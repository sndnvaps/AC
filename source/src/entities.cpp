// entities.cpp: map entity related functions (pickup etc.)

#include "cube.h"

vector<entity> ents;

char *entmdlnames[] = 
{
//FIXME : fix the "pickups" infront
	"pickups/pistolclips", "pickups/ammobox", "pickups/nades", "pickups/health", "pickups/kevlar", "pickups/akimbo",
};

int triggertime = 0;

void renderent(entity &e, char *mdlname, float z, float yaw, int frame = 0, int numf = 1, int basetime = 0, float speed = 10.0f)
{
	rendermodel(mdlname, frame, numf, 0, 1.1f, e.x, z+S(e.x, e.y)->floor, e.y, yaw, 0, false, 1.0f, speed, 0, basetime);
};

extern void newparticle(vec &o, vec &d, int fade, int type, int tex = -1);

void renderentities()
{
    if(lastmillis>triggertime+1000) triggertime = 0;
    loopv(ents)
    {
        entity &e = ents[i];
        if(e.type==MAPMODEL)
        {
            mapmodelinfo &mmi = getmminfo(e.attr2);
            if(!&mmi) continue;
			rendermodel(mmi.name, 0, 1, e.attr4, (float)mmi.rad, e.x, (float)S(e.x, e.y)->floor+mmi.zoff+e.attr3, e.y, (float)((e.attr1+7)-(e.attr1+7)%15), 0, false, 1.0f, 10.0f, mmi.snap);
        }
        else if(e.type==CTF_FLAG && m_ctf) // EDIT: AH
        {
            flaginfo &f = flaginfos[e.attr2];
            if(f.state==CTFF_STOLEN && f.thief)
            {
                if(f.thief == player1) continue;
                s_sprintfd(path)("pickups/flags/small_%s", rb_team_string(e.attr2));
                mapmodelinfo mmi = {10, 4, 0, 0, path};
                if(!&mmi) continue;
                rendermodel(mmi.name, 0, 1, 0, 0, f.thief->o.x, f.thief->o.z+0.3f+(sin(lastmillis/100.0f)+1)/10, f.thief->o.y, lastmillis/2.5f, 0, false, 0.6f, 120.0f, mmi.snap);
            }
            else
            {
                s_sprintfd(path)("pickups/flags/%s", rb_team_string(e.attr2));
                mapmodelinfo mmi = {10, 4, 0, 0, path};
                if(!&mmi) continue;
                rendermodel(mmi.name, 0, 7, 0, (float)mmi.rad, e.x, f.state==CTFF_INBASE ? (float)S(e.x, e.y)->floor : e.z, e.y, (float)((e.attr1+7)-(e.attr1+7)%15), 0, false, 1.0f, 120.0f, mmi.snap);
            };
        }
        else
        {
            if(OUTBORD(e.x, e.y)) continue;
            if(e.type!=CARROT)
            {
				if(!e.spawned) continue;
				if(e.type<I_CLIPS || e.type>I_AKIMBO) continue;
                renderent(e, entmdlnames[e.type-I_CLIPS], (float)(1+sin(lastmillis/100.0+e.x+e.y)/20), lastmillis/10.0f);
            }
			else switch(e.attr2)
            {			
				case 1:
				case 3:
					continue;
					
                case 2: 
                case 0:
					if(!e.spawned) continue;
					renderent(e, "carrot", (float)(1+sin(lastmillis/100.0+e.x+e.y)/20), lastmillis/(e.attr2 ? 1.0f : 10.0f));
					break;
					
                case 4: renderent(e, "switch2", 3,      (float)e.attr3*90, (!e.spawned && !triggertime) ? 1  : 0, (e.spawned || !triggertime) ? 1 : 2,  triggertime, 1050.0f);  break;
                case 5: renderent(e, "switch1", -0.15f, (float)e.attr3*90, (!e.spawned && !triggertime) ? 30 : 0, (e.spawned || !triggertime) ? 1 : 30, triggertime, 35.0f); break;
            }; 
        };
    };
};

itemstat itemstats[] =
{
	{1,	 1,   1,   S_ITEMAMMO},	  //knife dummy
    {16, 32,  72,  S_ITEMAMMO},   //pistol
    {14, 28,  21,  S_ITEMAMMO},   //shotgun
    {60, 90,  90,  S_ITEMAMMO},   //subgun
    {10, 20,  15,  S_ITEMAMMO},   //sniper
    {40, 60,  60,  S_ITEMAMMO},   //assault
    {2,  0,   2,   S_ITEMAMMO},   //grenade
	{33, 100, 100, S_ITEMHEALTH}, //health
    {50, 100, 100, S_ITEMARMOUR}, //armour
    {16, 0,   72,  S_ITEMPUP},    //powerup
};

void baseammo(int gun) { player1->ammo[gun] = itemstats[gun].add*2; };
// Added by Rick: baseammo for bots
void botbaseammo(int gun, dynent *d) { d->ammo[gun] = itemstats[gun].add*2; };
// End add

// these two functions are called when the server acknowledges that you really
// picked up the item (in multiplayer someone may grab it before you).

void radditem(int i, int &v, int t)
{
    itemstat &is = itemstats[t];
    ents[i].spawned = false;
    v += is.add;
    if(v>is.max) v = is.max;
    playsoundc(is.sound);
};

extern void weapon(int gun);

void realpickup(int n, dynent *d)
{
    switch(ents[n].type)
    {
        case I_CLIPS:  
            radditem(n, d->ammo[1], 1); 
            break;
    	case I_AMMO: 
            radditem(n, d->ammo[d->primary], d->primary); 
            break;
        case I_GRENADE: 
            radditem(n, d->mag[6], 6); 
            player1->thrownademillis = 0;
            break;
        case I_HEALTH:  
            radditem(n, d->health, 7);
            break;
        case I_ARMOUR:
            radditem(n, d->armour, 8);
            //d->hasarmour = true;
            break;
        case I_AKIMBO:
            d->akimbomillis = lastmillis+30000;
	        d->mag[GUN_PISTOL] = 16;
	        radditem(n, d->ammo[1], 9);
	        weaponswitch(GUN_PISTOL);
            break;
    };
};

// these functions are called when the client touches the item

void additem(int i, int &v, int spawnsec, int t)
{
      if(v<itemstats[t].max) 
      {
            addmsg(SV_ITEMPICKUP, "rii", i, spawnsec);
            ents[i].spawned = false;
      };
};

void pickup(int n, dynent *d)
{
    int np = 1;
    loopv(players) if(players[i]) np++;
    np = np<3 ? 4 : (np>4 ? 2 : 3);         // spawn times are dependent on number of players
    int ammo = np*2;
    switch(ents[n].type)
    {
        case I_CLIPS: 
            additem(n, d->ammo[1], ammo, 1);
            break;
    	case I_AMMO: 
            additem(n, d->ammo[d->primary], ammo, d->primary); 
            break;
    	case I_GRENADE: additem(n, d->mag[6], ammo, 6); break;
        case I_HEALTH:  additem(n, d->health,  np*5, 7); break;

        case I_ARMOUR:
            additem(n, d->armour, 20, 8);
            break;

        case I_AKIMBO:
            //additem(n, d->ammo[1], 60, 9);
            additem(n, d->akimbo, 60, 9);
            break;
            
        case CARROT:
            ents[n].spawned = false;
            triggertime = lastmillis;
            trigger(ents[n].attr1, ents[n].attr2, false);  // needs to go over server for multiplayer
            break;
            
        case LADDER:
        {
            d->onladder = true;
            break;
        };

        // EDIT: AH
        case CTF_FLAG:
        {
            int team = ents[n].attr2;
            flaginfo &f = flaginfos[team];
            if(f.state == CTFF_STOLEN) break;
            else if(team == rb_team_int(player1->team) && f.state == CTFF_DROPPED) 
            {
                addmsg(SV_FLAGRETURN, "ri", team);
                ents[n].spawned = false;
            }
            else if(team != rb_team_int(player1->team)) 
            {
                addmsg(SV_FLAGPICKUP, "ri", team);
                ents[n].spawned = false;
                f.thief = player1; // do this although we don't know if we picked the flag to avoid getting it after a possible respawn
                f.state = CTFF_STOLEN;
                f.pick_ack = false;
            }
            else if(team == rb_team_int(player1->team) && f.state == CTFF_INBASE && flaginfos[rb_opposite(team)].state == CTFF_STOLEN && flaginfos[rb_opposite(team)].thief == player1) addmsg(SV_FLAGSCORE, "ri", rb_opposite(team));
            break;
        };
    };
};

void checkitems()
{
    if(editmode) return;
    player1->onladder = false;
    loopv(ents)
    {
        entity &e = ents[i];
        if(e.type==NOTUSED) continue;
        if(e.type==LADDER)
        {
            if(OUTBORD(e.x, e.y)) continue;
            vec v = { e.x, e.y, player1->o.z };
            float dist1 = player1->o.dist(v);
            float dist2 = player1->o.z - (S(e.x, e.y)->floor+player1->eyeheight);
            if(dist1<1.5f && dist2<e.attr1) pickup(i, player1);
            continue;
        };
        
        if(!e.spawned) continue;
        if(OUTBORD(e.x, e.y)) continue;
        vec v = { e.x, e.y, S(e.x, e.y)->floor+player1->eyeheight };
        if(player1->o.dist(v)<2.5f) pickup(i, player1);
    };
};

void putitems(ucharbuf &p)            // puts items in network stream and also spawns them locally
{
    loopv(ents) if((ents[i].type>=I_CLIPS && ents[i].type<=I_AKIMBO) || ents[i].type==CARROT)
    {
		if(m_noitemsnade && ents[i].type!=I_GRENADE) continue;
		else if(m_pistol && ents[i].type==I_AMMO) continue;
        putint(p, i);
        ents[i].spawned = true;
    };
};

void resetspawns() 
{ 	
	loopv(ents) ents[i].spawned = false;
	if(m_noitemsnade || m_pistol)
		loopv(ents)
		{
			entity &e = ents[i];
			if(m_noitemsnade && e.type == I_CLIPS) e.type = I_GRENADE;
			else if(m_pistol && e.type==I_AMMO) e.type = I_CLIPS;
		}
};
void setspawn(int i, bool on) { if(i<ents.length() && i>=0) ents[i].spawned = on; };

void radd(dynent *d)
{
    loopi(NUMGUNS) if(d->nextprimary!=i) d->ammo[i] = 0;
    
	d->mag[GUN_KNIFE] = d->ammo[GUN_KNIFE] = 1;
    d->mag[GUN_GRENADE] = d->ammo[GUN_GRENADE] = 0;

    if(m_pistol || d->primary==GUN_PISTOL)  // pistol only mode
    {
		d->primary = GUN_PISTOL;
		d->ammo[GUN_PISTOL] = itemstats[GUN_PISTOL].max;
	}
	if(m_lss)
	{
		d->mag[d->primary] = d->mag[GUN_PISTOL] = 0;
		d->primary = GUN_KNIFE;
	}
	else if(!m_nogun)
	{
		d->ammo[GUN_PISTOL] = itemstats[GUN_PISTOL].start-magsize(GUN_PISTOL);
		d->mag[GUN_PISTOL] = magsize(GUN_PISTOL);
		d->ammo[d->primary] = itemstats[d->primary].start-magsize(d->primary);
		d->mag[d->primary] = magsize(d->primary);
    };

    if (d->hasarmour)
    {
        if(gamemode==m_arena) d->armour = 100;
	};

	d->gunselect = d->primary;
};

void item(int num)
{
      if (num>0 && num<7) player1->nextprimary = num;

      //if (num==6) conoutf("sorry, no nades yet");

      if (num==7) { player1->hasarmour=!player1->hasarmour;  /*toggle armour*/ };
};

COMMAND(item,ARG_1INT);

// Added by Rick
bool intersect(entity *e, vec &from, vec &to, vec *end) // if lineseg hits entity bounding box(entity version)
{
    mapmodelinfo &mmi = getmminfo(e->attr2);
    if(!&mmi || !mmi.h) return false;
    
    float lo = (float)(S(e->x, e->y)->floor+mmi.zoff+e->attr3);
    float hi = lo+mmi.h;
    vec v = to, w = { e->x, e->y, lo + (fabs(hi-lo)/2.0f) }, *p; 
    v.sub(from);
    w.sub(from);
    float c1 = w.dot(v);

    if(c1<=0) p = &from;
    else
    {
        float c2 = v.squaredlen();
        if(c2<=c1) p = &to;
        else
        {
            float f = c1/c2;
            v.mul(f);
            v.add(from);
            p = &v;
        };
    };
                        
    if (p->x <= e->x+mmi.rad
        && p->x >= e->x-mmi.rad
        && p->y <= e->y+mmi.rad
        && p->y >= e->y-mmi.rad
        && p->z <= hi
        && p->z >= lo)
     {
          if (end) *end = *p;
          return true;
     }
     return false;
};
// End add by Ricks
