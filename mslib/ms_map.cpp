#include "ms_map.h"

#include "ms_utils.h"

#include <iostream>

using namespace std;

namespace __MSLIB_NAMESPACE{

	bool	Map::Portal::render	()	{return true;}
	void	Map::Portal::draw	(SDL_Surface* surf,SDLAnim::PxPoint* p)	{}
	void	Map::Portal::free	()	{}
	__MSLIB_CTOR	Map::Portal::Portal(WZLib_SubProperty* parent):Object("__mslib_portal"){
		WZLib_Object* obj=(WZLib_Object*)parent;
		this->pt=_get(obj,"pt").intVal;
		this->pn=NSH(_get(obj,"pn").strVal);
		this->tm=_get(obj,"tm").intVal;
		this->tn=NSH(_get(obj,"tn").strVal);
		this->hi=_get(obj,"horizontalImpact").intVal;
		this->vi=_get(obj,"verticalImpact").intVal;
		this->script=NSH(_get(obj,"script").strVal);
		this->onlyOnce=_get(obj,"onlyOnce").intVal;
		this->hideTooltip=_get(obj,"hideTooltip").intVal;
		this->uDelay=_get(obj,"delay").intVal;
	}

	bool	Map::Foothold::render	()	{return true;}
	void	Map::Foothold::draw		(SDL_Surface* surf,SDLAnim::PxPoint* p)	{}
	void	Map::Foothold::free		()	{}
	__MSLIB_CTOR	Map::Foothold::Foothold(WZLib_SubProperty* parent):Object("__mslib_foothold"){
		WZLib_Object* obj=(WZLib_Object*)parent;
		this->x1=_get(obj,"x1").intVal;
		this->x2=_get(obj,"x2").intVal;
		this->y1=_get(obj,"y1").intVal;
		this->y2=_get(obj,"y2").intVal;
		this->force=_get(obj,"force").intVal;
		this->forbid=_get(obj,"forbidFallDown").intVal;
		this->id=StringConversion::convert<int,string>(obj->name);
		this->nextID=_get(obj,"next").intVal;
		this->prevID=_get(obj,"prev").intVal;
		this->layer=this->group=0;
		this->prevFH=this->nextFH=NULL;
		this->dir=this->len=0.0;
	}

	bool	Map::Ladder::render		()	{return true;}
	void	Map::Ladder::draw		(SDL_Surface* surf,SDLAnim::PxPoint* p)	{}
	void	Map::Ladder::free		()	{}
	__MSLIB_CTOR	Map::Ladder::Ladder(WZLib_SubProperty* parent):Object("__mslib_ladder"){
		WZLib_Object* obj=(WZLib_Object*)parent;
		this->x=_get(obj,"x").intVal;
		this->y1=_get(obj,"y1").intVal;
		this->y2=_get(obj,"y2").intVal;
		this->l=_get(obj,"l").intVal;
		this->page=_get(obj,"page").intVal;
		this->uf=_get(obj,"uf").intVal;
	}

	bool	Map::Tile::render		()	{return true;}
	void	Map::Tile::free			()	{}
	__MSLIB_CTOR	Map::Tile::Tile(WZLib_SubProperty* parent):Object("__mslib_tile"){
		WZLib_Object* obj=(WZLib_Object*)parent;
	}
	void Map::Tile::draw(SDL_Surface* surf,SDLAnim::PxPoint* p){
		Sprite::draw(surf,p);
	}

	__MSLIB_CTOR	Map::Map(WZLib_Image* mapImg):__MSLIB_NAMESPACE::Object("__mslib_map"){
		this->_mapImg=mapImg;
		if(this->_mapImg!=NULL)
			this->_dataSub=this->_mapImg->_sub;
		else
			this->_dataSub=NULL;
	}
	bool Map::render(){
		//footholds
		{
			WZLib_SubProperty* fhSub=(WZLib_SubProperty*)WZLib_Object_Get_n(WZLIB_OBJECT(this->_mapImg),"foothold");
			if(fhSub==NULL)
				return false; //zomgoodness
			for(int depth=0,depth_l=WZLib_Object_GetSize(WZLIB_OBJECT(fhSub));depth<depth_l;depth++){
				WZLib_SubProperty* thisDepth=(WZLib_SubProperty*)WZLib_Object_Get_i(WZLIB_OBJECT(fhSub),depth);
				for(int group=0,group_l=WZLib_Object_GetSize(WZLIB_OBJECT(thisDepth));group<group_l;group++){
					WZLib_SubProperty* thisGroup=(WZLib_SubProperty*)WZLib_Object_Get_i(WZLIB_OBJECT(thisDepth),group);
					for(int index=0,index_l=WZLib_Object_GetSize(WZLIB_OBJECT(thisGroup));index<index_l;index++){
						WZLib_SubProperty* thisFH=(WZLib_SubProperty*)WZLib_Object_Get_i(WZLIB_OBJECT(thisGroup),index);
						Map::Foothold* newFH=new Map::Foothold(thisFH);
						newFH->group=group;
						this->_footholds.push_back(newFH);
					}
				}
			}
			//link footholds
			for(unsigned int i=0,l=this->_footholds.size();i<l;i++){
				Foothold* fh=this->_footholds[i];
				int nextID=fh->nextID;
				int prevID=fh->prevID;
				if(nextID>0)
					fh->nextFH=this->_footholds[nextID-1];
				if(prevID>0)
					fh->prevFH=this->_footholds[prevID-1];
			}
		}
		//ladders
		{
			WZLib_SubProperty* ladderSub=(WZLib_SubProperty*)WZLib_Object_Get_n(WZLIB_OBJECT(this->_mapImg),"ladderRope");
			for(int i=0,l=WZLib_Object_GetSize(WZLIB_OBJECT(ladderSub));i<l;i++){
				WZLib_SubProperty* thisSub=(WZLib_SubProperty*)WZLib_Object_Get_i(WZLIB_OBJECT(ladderSub),i);
				Ladder* newL=new Ladder(thisSub);
				this->_ladders.push_back(newL);
			}
		}
		//tiles
		{
			for(int layer=0;layer<8;layer++){
				WZLib_SubProperty* thisLayer=(WZLib_SubProperty*)WZLib_Object_Get_n(WZLIB_OBJECT(this->_mapImg),StringConversion::convert<string,int>(layer).c_str());
				WZLib_PrimitiveProperty* tileSetProp=WZLIB_PRIMITIVEPROPERTY(WZLib_Object_Get_nv(WZLIB_OBJECT(thisLayer),"info","tS",NULL));
				if(tileSetProp==NULL)
					continue;
				string tileSet=tileSetProp->val.strVal;
				tileSet+=".img";
				WZLib_Image* tileSetImg=(WZLib_Image*)WZLib_Object_Get_nv(WZLIB_OBJECT(this->_mapImg->parentFile->_wzDir),"Tile",tileSet.c_str(),NULL);
				if(!WZLIB_OBJECT(tileSetImg)->parsed)
					WZLib_Image_Parse(tileSetImg);
				WZLib_SubProperty* tileSub=(WZLib_SubProperty*)WZLib_Object_Get_n(WZLIB_OBJECT(thisLayer),"tile");
				for(int tileNum=0,tileNum_l=WZLib_Object_GetSize(WZLIB_OBJECT(tileSub));tileNum<tileNum_l;tileNum++){
					WZLib_SubProperty* thisTile=(WZLib_SubProperty*)WZLib_Object_Get_n(WZLIB_OBJECT(tileSub),StringConversion::convert<string,int>(tileNum).c_str());
					string u=WZLIB_PRIMITIVEPROPERTY(WZLib_Object_Get_n(WZLIB_OBJECT(thisTile),"u"))->val.strVal;
					string no=StringConversion::convert<string,int>(WZLIB_PRIMITIVEPROPERTY(WZLib_Object_Get_n(WZLIB_OBJECT(thisTile),"no"))->val.intVal);
					WZLib_SubProperty* tileData=(WZLib_SubProperty*)WZLib_Object_Get_nv(WZLIB_OBJECT(tileSetImg),u.c_str(),no.c_str(),NULL);
					
					Tile* newTile=new Tile(thisTile);
					{
						SDLAnim::Frame* tileSurf=new SDLAnim::Frame();
						WZLib_PNGProperty* tilePNG=WZLIB_CANVASPROPERTY(tileData)->png;
						if(!tilePNG->pngParsed)
							WZLib_PNGProperty_Parse(tilePNG);
						tileSurf->setSurf(tilePNG->png);
						newTile->frames().push_back(tileSurf);
					}
					newTile->position()[0]=_get(WZLIB_OBJECT(thisTile),"x").intVal;
					newTile->position()[1]=_get(WZLIB_OBJECT(thisTile),"y").intVal;
					newTile->depth=Depth::Layer;
					newTile->depth=layer;
					newTile->depth=Depth::Tiles;
					newTile->depth=_get(WZLIB_OBJECT(tileData),"z").intVal;
					this->_objects.insert(newTile);
				}
			}
		}
		return true;
	}
	void Map::draw(SDL_Surface *surf,SDLAnim::PxPoint* p){
		for(std::set<__MSLIB_NAMESPACE::Object*,__MSLIB_NAMESPACE::Depth>::iterator i=this->_objects.begin(),l=this->_objects.end();i!=l;i++)
			(*i)->draw(surf,p);
	}
	void Map::update(){
		for(std::set<__MSLIB_NAMESPACE::Object*,__MSLIB_NAMESPACE::Depth>::iterator i=this->_objects.begin(),l=this->_objects.end();i!=l;i++)
			(*i)->update();
	}
	void Map::free(){
		return; //harrumph
	}
}
