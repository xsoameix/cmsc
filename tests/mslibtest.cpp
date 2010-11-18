#include <iostream>
#include <string>

#include "wzlibc.h"
#include "ms_map.h"

#include "SDLAnim/SDLAnim.h"

using namespace std;

int main(int argc,char* argv[]){
	//Map.wz -> Map -> Map0 -> 001010002.img -> foothold -> 1 -> 0 -> 2
	{
		SDLAnim::ScreenStats ss;
		ss.flags=SDL_HWSURFACE;
		ss.screenDepth=32;
		ss.screenHeight=600;
		ss.screenWidth=800;
		SDLAnim::Engine::Init(&ss,SDL_HWSURFACE);
	}
	WZLib_File* mapWZ=WZLib_File_Open("../wzs/Map.wz");
	if(mapWZ==NULL){
		cout<<"can't find Map.wz."<<endl;
		goto abort;
	}
	WZLib_File_Parse(mapWZ);
	WZLib_Image* mapImg=(WZLib_Image*)WZLib_Object_Get_nv(WZLIB_OBJECT(mapWZ->_wzDir),"Map","Map0","001010002.img",NULL);
	if(mapImg==NULL){
		cout<<"can't find Map img."<<endl;
		goto abort;
	}
	WZLib_Image_Parse(mapImg);
	MS::Map* map=new MS::Map(mapImg);
	map->render();
	SDLAnim::Engine::AddObject(map);
	{
		bool running=true;
		SDL_Event* ev=new SDL_Event();
		while(running){
			while(SDL_PollEvent(ev)){
				switch(ev->type){
				case SDL_QUIT:
					running=false;
					break;
				case SDL_KEYDOWN:
					if(ev->key.keysym.sym==SDLK_ESCAPE)
						running=false;
					break;
				}
			SDLAnim::Engine::Sync();
			}
		}
	}
abort:
	system("pause");
	return 0;
}
