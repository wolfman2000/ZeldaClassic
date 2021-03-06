//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------
//
//Copyright (C) 2016 Zelda Classic Team
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include "sprite.h"
#include "entityPtr.h"
#include "zdefs.h"
#include "tiles.h"
#include "angelscript/scriptData.h"

extern bool get_debug();
extern bool halt;
extern bool show_sprites;
extern bool show_hitboxes;
extern bool is_zquest();
extern void debugging_box(int x1, int y1, int x2, int y2);
extern BITMAP* scriptDrawingTarget;

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

sprite::sprite():
    scriptData(0),
    toBeDeleted(false),
    ref(new EntityRef())
{
    uid = getNextUID();
    x=y=z=tile=shadowtile=cs=flip=c_clk=clk=xofs=yofs=zofs=hxofs=hyofs=fall=0;
    txsz=1;
    tysz=1;
    id=-1;
    hxsz=hysz=16;
    hzsz=1;
    yofs=playing_field_offset;
    dir=down;
    angular=canfreeze=false;
    drawstyle=0;
    extend=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    misc=0;
    for(int i=0; i<16; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
}

sprite::sprite(sprite const & other):
    x(other.x),
    y(other.y),
    z(other.z),
    fall(other.fall),
    tile(other.tile),
    shadowtile(other.shadowtile),
    cs(other.cs),
    flip(other.flip),
    c_clk(other.c_clk),
    clk(other.clk),
    misc(other.misc),
    xofs(other.xofs),
    yofs(other.yofs),
    zofs(other.zofs),
    hxofs(other.hxofs),
    hyofs(other.hyofs),
    hxsz(other.hxsz),
    hysz(other.hysz),
    hzsz(other.hzsz),
    txsz(other.txsz),
    tysz(other.tysz),
    id(other.id),
    dir(other.dir),
    angular(other.angular),
    canfreeze(other.canfreeze),
    angle(other.angle),
    lasthit(other.lasthit),
    lasthitclk(other.lasthitclk),
    drawstyle(other.drawstyle),
    extend(other.extend),
    scriptData(0),
    toBeDeleted(false),
    ref(new EntityRef())
{
    uid = getNextUID();
    for(int i=0; i<16; i++) miscellaneous[i] = other.miscellaneous[i];
    
    scriptcoldet = other.scriptcoldet;
}

sprite::sprite(fix X,fix Y,int T,int CS,int F,int Clk,int Yofs):
    x(X),y(Y),tile(T),cs(CS),flip(F),clk(Clk),yofs(Yofs),
    scriptData(0),
    toBeDeleted(false),
    ref(new EntityRef())
{
    uid = getNextUID();
    hxsz=hysz=16;
    hxofs=hyofs=xofs=0;
    txsz=1;
    tysz=1;
    id=-1;
    dir=down;
    angular=canfreeze=false;
    extend=0;
    for(int i=0; i<16; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
    drawstyle=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    hzsz=1;
    misc=0;
    c_clk=0;
    shadowtile=0;
}

sprite::~sprite()
{
    ref->onEntityDelete();
    delete scriptData;
}

long sprite::getNextUID()
{
    static long nextid = 0;
    return nextid++;
}

void sprite::draw2(BITMAP *)                            // top layer for special needs
{
}

void sprite::drawcloaked2(BITMAP *)                     // top layer for special needs
{
}

bool sprite::animate(int)
{
    ++c_clk;
    return false;
}
int sprite::real_x(fix fx)
{
    int rx=fx.v>>16;
    
    switch(dir)
    {
    case 9:
    case 13:
        if(fx.v&0xFFFF)
            ++rx;
            
        break;
    }
    
    return rx;
}

int sprite::real_y(fix fy)
{
    return fy.v>>16;
}

int sprite::real_z(fix fz)
{
    return fz.v>>16;
}

bool sprite::hit(sprite *s)
{
    if(!(scriptcoldet&1)) return false;
    
    if(id<0 || s->id<0 || clk<0) return false;
    
    if(halt)
    {
    }
    
    return hit(s->x+s->hxofs,s->y+s->hyofs,s->z+s->zofs,s->hxsz,s->hysz,s->hzsz);
}

bool sprite::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    if(!(scriptcoldet&1)) return false;
    
    if(id<0 || clk<0) return false;
    
    return tx+txsz2>x+hxofs &&
           ty+tysz2>y+hyofs &&
           tz+tzsz2>z+zofs &&
           
           tx<x+hxofs+hxsz &&
           ty<y+hyofs+hysz &&
           tz<z+zofs+hzsz;
}

int sprite::hitdir(int tx,int ty,int txsz2,int tysz2,int dir2)
{
    if(!(scriptcoldet&1)) return 0xFF;
    
    int cx1=x+hxofs+(hxsz>>1);
    int cy1=y+hyofs+(hysz>>1);
    int cx2=tx+(txsz2>>1);
    int cy2=ty+(tysz2>>1);
    
    if(dir2>=left && abs(cy1-cy2)<=8)
        return (cx2-cx1<0)?left:right;
        
    return (cy2-cy1<0)?up:down;
}

void sprite::move(fix dx,fix dy)
{
    x+=dx;
    y+=dy;
}

void sprite::move(fix s)
{
    if(angular)
    {
        x += cos(angle)*s;
        y += sin(angle)*s;
        return;
    }
    
    switch(dir)
    {
    case 8:
    case up:
        y-=s;
        break;
        
    case 12:
    case down:
        y+=s;
        break;
        
    case 14:
    case left:
        x-=s;
        break;
        
    case 10:
    case right:
        x+=s;
        break;
        
    case 15:
    case l_up:
        x-=s;
        y-=s;
        break;
        
    case 9:
    case r_up:
        x+=s;
        y-=s;
        break;
        
    case 13:
    case l_down:
        x-=s;
        y+=s;
        break;
        
    case 11:
    case r_down:
        x+=s;
        y+=s;
        break;
        
    case -1:
        break;
    }
}

void sprite::draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,0,0,cs,flip);
                overtile16(temp,tile,0,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,0,0,cs,flip,128);
                overtiletranslucent16(temp,tile,0,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,0,0,flip);
                overtilecloaked16(temp,tile,0,16,flip);
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,16,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip);
                overtile16(temp,tile,16,16,cs,flip);
                overtile16(temp,tile-(flip?-1:1),0,16,cs,flip);
                overtile16(temp,tile+(flip?-1:1),32,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,16,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip,128);
                overtiletranslucent16(temp,tile,16,16,cs,flip,128);
                overtiletranslucent16(temp,tile-(flip?-1:1),0,16,cs,flip,128);
                overtiletranslucent16(temp,tile+(flip?-1:1),32,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,16,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,flip);
                overtilecloaked16(temp,tile,16,16,flip);
                overtilecloaked16(temp,tile-(flip?-1:1),0,16,flip);
                overtilecloaked16(temp,tile+(flip?-1:1),32,16,flip);
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            
            switch(flip)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,tile,sx,sy,cs,flip);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,tile,sx,sy,cs,flip,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,tile,sx,sy,flip);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].tile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((fix)sx,(fix)sy,wpnsbuf[extend].tile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
            if(BSZ)
            {
                if(clk>=-10)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-5)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            else
            {
                if(clk>=-12)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-6)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::draw8(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        switch(drawstyle)
        {
        case 0:                                               //normal
            overtile8(dest,tile,sx,sy,cs,flip);
            break;
            
        case 1:                                               //phantom
            overtiletranslucent8(dest,tile,sx,sy,cs,flip,128);
            break;
        }
    }
}

void sprite::drawcloaked(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        overtilecloaked16(dest,tile,sx,sy,flip);
    }
    else
    {
        int t  = wpnsbuf[iwSpawn].tile;
        int cs2 = wpnsbuf[iwSpawn].csets&15;
        
        if(BSZ)
        {
            if(clk>=-10) ++t;
            
            if(clk>=-5) ++t;
        }
        else
        {
            if(clk>=-12) ++t;
            
            if(clk>=-6) ++t;
        }
        
        overtile16(dest,t,x,sy,cs2,0);
    }
    
    if(get_debug() && key[KEY_O])
        rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
}

void sprite::drawshadow(BITMAP* dest,bool translucent)
{
    if(extend == 4 || shadowtile==0 || id<0)
    {
        return;
    }
    
    int shadowcs = wpnsbuf[iwShadow].csets & 0xFFFF;
    int shadowflip = wpnsbuf[iwShadow].misc & 0xFF;
    
    int sx = real_x(x+xofs)+(txsz-1)*8;
    int sy = real_y(y+yofs+(tysz-1)*16);
    
    if(clk>=0)
    {
        if(translucent)
        {
            overtiletranslucent16(dest,shadowtile,sx,sy,shadowcs,shadowflip,128);
        }
        else
        {
            overtile16(dest,shadowtile,sx,sy,shadowcs,shadowflip);
        }
    }
}

asIScriptObject* sprite::getScriptObject()
{
    return scriptData->getObject();
}

void sprite::scriptDraw()
{
    sprite::draw(scriptDrawingTarget);
}

void sprite::scriptDrawCloaked()
{
    sprite::drawcloaked(scriptDrawingTarget);
}

/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/


sprite_list::sprite_list() : count(0) {}
void sprite_list::clear()
{
	for(int i(0); i < count; i++)
	{
		delete sprites[i];
	}

	count = 0;
}

sprite *sprite_list::spr(int index)
{
    if((unsigned)index >= (unsigned)count)
        return NULL;
        
    return sprites[index];
}

bool sprite_list::add(sprite *s)
{
    if(count >= SLMAX)
    {
        delete s;
        return false;
    }
    
    uids[count] = s->getUID();
    sprites[count] = s;
	++count;

    //checkConsistency();
    return true;
}

bool sprite_list::addAtFront(sprite *s)
{
    if(count>=SLMAX)
    {
        delete s;
        return false;
    }
    
	// Reflect sign bit to reverse direction of integer.
	// This is needed to keep uids sorted.
	s->uid = -s->uid;

	::memmove(sprites + 1, sprites, count * sizeof(sprite*));
	::memmove(uids + 1, uids, count * sizeof(long));

	uids[0] = s->getUID();
	sprites[0] = s;
	++count;

    return true;
}


bool sprite_list::addExisting(sprite *s)
{
	s->uid = s->getNextUID();
	return add(s);
}


bool sprite_list::remove(sprite *s)
// removes pointer from list but doesn't delete it
{
	for(int i(0); i != count; ++i)
	{
		if(sprites[i] == s)
		{
			int n = count - i - 1;
			::memcpy(sprites + i, sprites + i + 1, n * sizeof(sprite*));
			::memcpy(uids + i, uids + i + 1, n * sizeof(long));

			--count;
			return true;
		}
	}

    //checkConsistency();
    return false;
}

fix sprite_list::getX(int j)
{
    if((j>=count)||(j<0))
    {
        return (fix)1000000;
    }
    
    return sprites[j]->x;
}

fix sprite_list::getY(int j)
{
    if((j>=count)||(j<0))
    {
        return (fix)1000000;
    }
    
    return sprites[j]->y;
}

int sprite_list::getID(int j)
{
    if((j>=count)||(j<0))
    {
        return -1;
    }
    
    return sprites[j]->id;
}

int sprite_list::getMisc(int j)
{
    if((j>=count)||(j<0))
    {
        return -1;
    }
    
    return sprites[j]->misc;
}

bool sprite_list::del(int j)
{
    if((unsigned)j >= (unsigned)count)
        return false;

	delete sprites[j];

	int n = count - j - 1;
	::memcpy(sprites + j, sprites + j + 1, n * sizeof(sprite*));
	::memcpy(uids + j, uids + j + 1, n * sizeof(long));
    
    --count;
    //checkConsistency();
    return true;
}

void sprite_list::draw(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
        {
            sprites[i]->draw(dest);
        }
        
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
        {
            sprites[i]->draw(dest);
        }
        
        break;
    }
}

void sprite_list::drawshadow(BITMAP* dest,bool translucent, bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
    }
}

void sprite_list::draw2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->draw2(dest);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->draw2(dest);
            
        break;
    }
}

void sprite_list::drawcloaked2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawcloaked2(dest);
            
        break;
        
    case false:
    
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawcloaked2(dest);
            
        break;
    }
}

void sprite_list::animate()
{
    int i=0;
    
    while(i<count)
    {
        if(sprites[i]->isMarkedForDeletion())
        {
            del(i);
            continue;
        }
        
        if(!(freeze_guys && sprites[i]->canfreeze))
        {
            if(sprites[i]->animate(i))
            {
                del(i);
                --i;
            }
        }
        
        ++i;
    }
}

void sprite_list::check_conveyor()
{
    int i=0;
    
    while(i<count)
    {
        sprites[i]->check_conveyor();
        ++i;
    }
}

int sprite_list::Count()
{
    return count;
}

int sprite_list::hit(sprite *s)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(s))
            return i;
            
    return -1;
}

int sprite_list::hit(int x,int y,int z, int xsize, int ysize, int zsize)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(x,y,z,xsize,ysize,zsize))
            return i;
            
    return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id, int mask)
{
    int c=0;
    
    for(int i=0; i<count; i++)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            ++c;
        }
    }
    
    return c;
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id, int mask)
{
    for(int i=0; i<count; i++)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            return i;
        }
    }
    
    return -1;
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id, int mask)
{
    for(int i=count-1; i>=0; i--)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            return i;
        }
    }
    
    return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id)
{
    return idCount(id,0xFFFF);
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id)
{
    return idFirst(id,0xFFFF);
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id)
{
    return idLast(id,0xFFFF);
}

sprite * sprite_list::getByUID(long uid) const
{
	if(count)
	{
		const long* p = std::lower_bound(uids, uids + count, uid);
		if(p != (uids + count) && *p == uid)
			return sprites[(int)(p - uids)];
	}
        
    return NULL;
}

void sprite_list::checkConsistency()
{
    for(int i=0; i<count; i++)
        assert(sprites[i] == getByUID(sprites[i]->getUID()));

	for(int i=0; i<count-1; i++)
		assert(uids[i] < uids[i + 1]);
}

bool sprite_list::isValidUID(long uid) const
{
	return getByUID(uid) != NULL;
}


/**********************************/
/********** Moving Block **********/
/**********************************/

movingblock::movingblock() : sprite()
{
    id=1;
}

void movingblock::draw(BITMAP *dest)
{
    if(clk)
    {
        //    sprite::draw(dest);
        overcombo(dest,real_x(x+xofs),real_y(y+yofs),bcombo ,cs);
    }
}

/*** end of sprite.cc ***/

