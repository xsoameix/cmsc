#include <errno.h>

#include "wzlibc.h"

#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#ifndef _WIN32
int max(int x,int y){
	if(x>y)
		return x;
	return y;
}
#endif

void play(WZLib_Object* obj);

void quitSDL(){
	//Mix_Quit();
	SDL_Quit();
}

void tokenize(const char* str,const char* delim,char*** outTokens,int* numTokens){
	const int guessSize=2;
	char** tokens=(char**)malloc(sizeof(char*)*guessSize);
	int index=0;
	int tokenStart=0;
	int tokenIndex=0;
	int tokensCap=guessSize;
	int len=strlen(str);
	for(index=0;index<=len;index++){
		if(str[index]==delim[0]){ /*match*/
			int strlen=0;
			if(index==tokenStart){
				tokenStart++; /*in case it's two delims after the other*/
				continue;
			}
			/*do we need to expand tokens?*/
			if(tokenIndex+1>=tokensCap){
				tokens=(char**)realloc(tokens,sizeof(char*)*(tokensCap+guessSize));
				tokensCap+=guessSize;
			}
			strlen=index-tokenStart;
			tokens[tokenIndex]=(char*)malloc(sizeof(char*)*(strlen+1));
			memcpy(tokens[tokenIndex],str+(sizeof(char)*tokenStart),strlen+1);
			tokens[tokenIndex][strlen]=0; /*null terminate pl0x*/
			tokenIndex++;
			tokenStart=index+1;
		}
	}
	if(str[tokenStart]!=0){
		int strlen=0;
		if(tokenIndex+1>=tokensCap){
			tokens=(char**)realloc(tokens,sizeof(char*)*(tokensCap+guessSize));
			tokensCap+=guessSize;
		}
		strlen=index-tokenStart;
		tokens[tokenIndex]=(char*)malloc(sizeof(char*)*(strlen+1));
		memcpy(tokens[tokenIndex],str+(sizeof(char)*tokenStart),strlen+1);
		tokens[tokenIndex][strlen]=0; /*null terminate pl0x*/
		tokenIndex++;
		tokenStart=index+1;
	}
	if(tokenIndex<tokensCap)
		tokens=(char**)realloc(tokens,sizeof(char*)*tokenIndex);
	*outTokens=tokens;
	*numTokens=tokenIndex;
}

char* getFileName(char* path){
	int startIndex=0;
	int index=strlen(path)-1;
	char* ret=path;
	for(;index>=0;index--){
		if(path[index]=='/' || path[index]=='\\'){
			ret=(char*)malloc(sizeof(char)*(strlen(path)-index));
			memcpy(ret,path+(sizeof(char)*index+1),strlen(path)-index+1);
			break;
		}
	}
	return ret;
}

WZLib_Object* currentObj=NULL;
WZLib_File* currentFile=NULL;

#define INM(str,longForm) (strcmp(str,longForm)==0 || tolower(str[0])==longForm[0])
#define INA(cmd,found,needed) if(found<needed){printf("%s needs %d arguments, found %d.\n",cmd,needed,found);return 0;}
#define INE(cmd,message) {printf("%s: %s\n",cmd,message);return 0;}

int interpret(const char* cmd){
	char** tokens=NULL;
	int numTokens;
	tokenize(cmd," ",&tokens,&numTokens);
	if(numTokens<1){
		printf("No input lulz.\n");
		return 0;
	}
	if(INM(tokens[0],"quit")){
		interpret("c");
		printf("Cya.\n");
		return 1;
	}
	if(INM(tokens[0],"play")){
		play(currentObj);
	}
	if(INM(tokens[0],"resolve")){
		WZLib_Object* child=NULL;
		INA("resolve",numTokens,2);
		if(currentObj==NULL)
			INE("resolve","currentObj is NULL");
		child=WZLib_Object_Get_n(currentObj,tokens[1]);
		if(child==NULL)
			INE("resolve","couldn't find argument");
		if(child->type!=WZLib_ObjectType_Property && WZLIB_PROPERTY(child)->propType!=WZLib_Prop_UOL)
			INE("resolve","bad argument type");
		child=WZLib_UOLProperty_Resolve(WZLIB_PRIMITIVEPROPERTY(child));
		if(child==NULL)
			INE("resolve","UOL resolved to NULL");
		printf("resolving to: %s\n",child->name);
		currentObj=child;
	}
	if(INM(tokens[0],"open")){
		INA("open",numTokens,2);
		if(currentFile!=NULL)
			interpret("c");
		currentObj=WZLib_File_Open(tokens[1]);
		if(currentObj==NULL)
			INE("open","invalid file path");
		currentObj->name=getFileName(tokens[1]);
		currentFile=currentObj;
		WZLib_File_Parse((WZLib_File*)currentObj);
		printf("opened %s, v%d, %d bytes, ",currentObj->name,WZLIB_FILE(currentObj)->_fileVersion,WZLIB_FILE(currentObj)->_header->fileSize);
		printf("(%s)\n",WZLIB_FILE(currentObj)->_header->copyright);
		currentObj=WZLIB_FILE(currentObj)->_wzDir;
	}
	if(INM(tokens[0],"close")){
		if(currentFile==NULL)
			INE("close","already closed");
		printf("closing %s\n",WZLIB_OBJECT(currentFile)->name);
		WZLib_File_Close(WZLIB_FILE(currentFile));
		currentObj=NULL;
		currentFile=NULL;
	}
	if(INM(tokens[0],"list")){
		int size;
		int i=0;
		if(currentObj==NULL)
			INE("list","no currentObj");
		size=WZLib_Object_GetSize(currentObj);
		if(size==-1)
			INE("list","currentObj is wrong type");
		for(i=0;i<size;i++){
			WZLib_Object* obj=WZLib_Object_Get_i(currentObj,i);
			if(obj->type!=WZLib_ObjectType_Property)
				printf("%s :: ",WZLib_ObjectType_GetName(obj->type));
			else
				printf("%s :: ",WZLib_PropertyType_GetName(WZLIB_PROPERTY(obj)->propType));
			printf("%s",obj->name);
			if(obj->type==WZLib_ObjectType_Property && WZLIB_SPROPTYPE(obj)==WZLib_Prop_Primitive){
				printf(" = ");
				switch(WZLIB_PROPERTY(obj)->propType){
					case WZLib_Prop_Null:
						printf("NULL");
						break;
					case WZLib_Prop_UnsignedShort:
					case WZLib_Prop_CompressedInt:
						printf("%d",WZLIB_PRIMITIVEPROPERTY(obj)->val.intVal);
						break;
					case WZLib_Prop_Float:
					case WZLib_Prop_Double:
						printf("%f",WZLIB_PRIMITIVEPROPERTY(obj)->val.dblVal);
						break;
					case WZLib_Prop_UOL:
					case WZLib_Prop_String:
						printf("%s",WZLIB_PRIMITIVEPROPERTY(obj)->val.strVal);
						break;
					case WZLib_Prop_Vector:
						printf("[%d,%d]",WZLIB_PRIMITIVEPROPERTY(obj)->val.vecVal[0],WZLIB_PRIMITIVEPROPERTY(obj)->val.vecVal[1]);
						break;
				}
				printf("\n");
			}else
				printf("\n");
		}
	}
	if(INM(tokens[0],"down")){
		WZLib_Object* dobj=NULL;
		INA("down",numTokens,2);
		dobj=WZLib_Object_Get_n(currentObj,tokens[1]);
		if(dobj==NULL)
			INE("down","could not find argument");
		currentObj=dobj;
		if(currentObj->type==WZLib_ObjectType_Image){
			WZLib_Image_Parse((WZLib_Image*)currentObj);
			WZLib_Image_ResolveUOLs((WZLib_Image*)currentObj);
		}
	}
	if(INM(tokens[0],"keyset")){
		int ret=0;
		INA("keyset",numTokens,2);
		if((ret=WZLib_Key_LoadKeyFromFile(tokens[1]))!=0)
			INE("keyset","error loading key from file");
	}
	if(INM(tokens[0],"up")){
		if(currentObj==NULL)
			INE("up","currentObj is NULL");
		if(currentObj->parent==NULL)
			INE("up","currentObj has no parent");
		if(currentObj->type==WZLib_ObjectType_Image)
			WZLib_Image_Unparse((WZLib_Image*)currentObj);
		currentObj=currentObj->parent;
		if(currentObj->parent==NULL)
			return 0;
		if(currentObj->parent->type==WZLib_ObjectType_Image){
			currentObj=currentObj->parent;
			return 0;
		}
		if(currentObj->parent->type==WZLib_ObjectType_Directory && 1==0){
			currentObj=currentObj->parent;
			return 0;
		}
	}
	if(INM(tokens[0],"eval")){
		INA("eval",numTokens,2);
		system(tokens[1]);
	}
	if(INM(tokens[0],"memtest")){
		WZLib_Object* obj=NULL;
		WZLib_Image* img=NULL;
		INA("memtest",numTokens,2);
		if(currentObj==NULL || ((obj=WZLib_Object_Get_n(currentObj,tokens[1]))==NULL))
			INE("memtest","fail");
		img=(WZLib_Image*)obj;
		{
			int i=0,l=30;
			for(;i<l;i++){
				printf("%d of %d\n",i+1,l);
				WZLib_Image_Parse(img);
				WZLib_Image_Unparse(img);
			}
		}
	}
	if(INM(tokens[0],"show")){
		WZLib_Object* obj=NULL;
		SDL_Surface* screen=NULL;
		WZLib_PNGProperty* png=NULL;
		SDL_Event ev;
		int running=1;
		INA("show",numTokens,2);
		obj=WZLib_Object_Get_n(currentObj,tokens[1]);
		if(obj==NULL)
			INE("show","could not find argument");
		if(obj->type!=WZLib_ObjectType_Property || (WZLIB_PROPERTY(obj)->propType!=WZLib_Prop_Canvas && WZLIB_PROPERTY(obj)->propType!=WZLib_Prop_MP3))
			INE("show","argument is not WZCanvasProperty or WZSoundProperty");
		if(WZLIB_PROPERTY(obj)->propType==WZLib_Prop_Canvas){
			SDL_Init(SDL_INIT_EVERYTHING);
			png=WZLIB_CANVASPROPERTY(obj)->png;
			screen=SDL_SetVideoMode(png->width,png->height,32,SDL_HWSURFACE|SDL_DOUBLEBUF);
			if(!png->pngParsed)
				WZLib_PNGProperty_Parse(png);
			while(running){
				while(SDL_PollEvent(&ev)){
					switch(ev.type){
						case SDL_QUIT:
							running=0;
							break;
						case SDL_KEYDOWN:
							if(ev.key.keysym.sym==SDLK_ESCAPE)
								running=0;
							break;
					}
				}
				SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,255,0,255));
				SDL_BlitSurface(png->png,NULL,screen,NULL);
				SDL_Flip(screen);
			}
			SDL_Quit();
		}else if(WZLIB_PROPERTY(obj)->propType==WZLib_Prop_MP3){
			SDL_Init(SDL_INIT_EVERYTHING);
			Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,1024);
			WZLib_SoundProperty_Play((WZLib_SoundProperty*)obj);
			Mix_HookMusicFinished(quitSDL);
			printf("Playing %s, %d secs, %d bytes\n",WZLIB_OBJECT(obj)->name,((WZLib_SoundProperty*)obj)->lenMS/1000,((WZLib_SoundProperty*)obj)->_lenData);
		}
	}
	return 0;
}

void printInh(WZLib_Object* obj){
	/*//Map.wz -> Back -> midForest -> back -> 0.png*/
	char* res=(char*)malloc(sizeof(char)*2000);/*large size is lulz*/
	WZLib_Object** inh=(WZLib_Object**)malloc(sizeof(WZLib_Object*)*100);/*large size is lulz*/
	int inhLength=0;
	WZLib_Object* oobj=obj;
	/*prints parents all the way up to NULL*/
	memset(res,0,sizeof(char)*2000);
	memset(inh,0,sizeof(WZLib_Object*)*100);
	/*up*/
	do{
		if(obj->parent!=NULL && obj->parent->type==WZLib_ObjectType_Image)
			obj=obj->parent;
		inh[inhLength]=obj;
		obj=obj->parent;
		inhLength++;
	}while(obj!=NULL);
	/*and back down*/
	{
		int i=inhLength-1;
		for(i=inhLength-1;i>=0;i--){
			res=strcat(res,inh[i]->name);
			res=strcat(res,"::");
		}
		/*drop last ::*/
		res[strlen(res)-2]=0;
	}
	printf("%s (%d)> ",res,oobj->objectID);
	free(res);
	free(inh);
}

int main(int argc,char* argv[]){
	const int inSize=200;
	char* inStr=(char*)malloc(sizeof(char)*inSize);
	while(1){
		if(currentObj!=NULL)
			printInh(currentObj);
		else
			printf("(null)> ");
		fgets(inStr,inSize,stdin);
		inStr[strlen(inStr)-1]=0;
		if(interpret(inStr))
			break;
	}
	return 0;
}

void play(WZLib_Object* obj){
	int size=WZLib_Object_GetSize(obj);
	int i=0;
	int listIndex=0;
	int matchPoint[2]={0,0};
	WZLib_CanvasProperty** list=NULL;
	int* delays=NULL;
	int** origins=NULL;
	SDL_Surface* screen=NULL;
	if(size<=0)
		return;
	list=(WZLib_CanvasProperty**)malloc(sizeof(WZLib_CanvasProperty*)*size);
	delays=(int*)malloc(sizeof(int)*size);
	origins=(int**)malloc(sizeof(int*)*size);
	memset(list,0,sizeof(WZLib_CanvasProperty*)*size);
	for(i=0;i<size;i++){
		WZLib_Object* child=WZLib_Object_Get_i(obj,i);
		if(child==NULL)
			break;
		if(child->type==WZLib_ObjectType_Property && WZLIB_PROPERTY(child)->propType==WZLib_Prop_Canvas){
			list[listIndex]=WZLIB_CANVASPROPERTY(child);
			listIndex++;
		}
	}
	if(listIndex==0){
		printf("play: could not find any canvas children.\n");
		return;
	}
	{
		int width=0,height=0;
		/*pass 1*/
		for(i=0;i<listIndex;i++){
			width=max(width,list[i]->png->width);
			height=max(list[i]->png->height,height);
			if(height==list[i]->png->height){
				WZLib_PrimitiveProperty* origin=(WZLib_PrimitiveProperty*)WZLib_SubProperty_Get_n(WZLIB_SUBPROPERTY(list[i]),"origin");
				if(origin==NULL)
					continue;
				matchPoint[0]=origin->val.vecVal[0];
				matchPoint[1]=origin->val.vecVal[1];
			}
		}
		/*pass 2*/
		{
			int mpx=matchPoint[0];
			int mpy=matchPoint[1];
			for(i=0;i<listIndex;i++){
				WZLib_PrimitiveProperty* origin=WZLIB_PRIMITIVEPROPERTY(WZLib_SubProperty_Get_n(WZLIB_SUBPROPERTY(list[i]),"origin"));
				int drawx;
				int drawy;
				if(origin==NULL)
					continue;
				drawx=matchPoint[0]-origin->val.vecVal[0];
				drawy=matchPoint[1]-origin->val.vecVal[1];
				if(drawx<0)
					mpx=max(mpx,origin->val.vecVal[0]);
				if(drawy<0)
					mpy=max(mpy,origin->val.vecVal[1]);
				width=max(width,drawx+list[i]->png->width);
				height=max(height,drawy+list[i]->png->height);
				origins[i]=(int*)malloc(sizeof(int)*2);
				origins[i][0]=origin->val.vecVal[0];
				origins[i][1]=origin->val.vecVal[1];
				/*repurpose origin*/
				origin=WZLIB_PRIMITIVEPROPERTY(WZLib_SubProperty_Get_n(WZLIB_SUBPROPERTY(list[i]),"delay"));
				if(origin!=NULL)
					delays[i]=origin->val.intVal;
				else
					delays[i]=100;
			}
			matchPoint[0]=mpx;
			matchPoint[1]=mpy;
		}
		screen=SDL_SetVideoMode(width,height,32,SDL_HWSURFACE);
	}
	for(i=0;i<listIndex;i++){
		printf("Rendering sprite %d/%d\n",i+1,listIndex);
		if(!list[i]->png->pngParsed)
			WZLib_PNGProperty_Parse(list[i]->png);
	}
	{
		SDL_Event ev;
		int running=1;
		unsigned int lastBlit=0;
		SDL_Rect br;
		br.x=br.y=0;
		i=0;
		while(running){
			while(SDL_PollEvent(&ev)){
				switch(ev.type){
					case SDL_QUIT:
						running=0;
						break;
					case SDL_KEYDOWN:
						if(ev.key.keysym.sym==SDLK_ESCAPE)
							running=0;
						break;
				}
			}
			if(SDL_GetTicks()-lastBlit > delays[i]){
				i++;
				lastBlit=SDL_GetTicks();
			}
			if(i>=listIndex)
				i=0;
			br.x=matchPoint[0]-origins[i][0];
			br.y=matchPoint[1]-origins[i][1];
			SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,255,0,255));
			SDL_BlitSurface(list[i]->png->png,NULL,screen,&br);
			SDL_Flip(screen);
		}
	}
	for(i=0;i<listIndex;i++){
		printf("Freeing sprite %d/%d\n",i+1,listIndex);
		WZLib_PNGProperty_Unparse(list[i]->png);
	}
	SDL_Quit();
}
