#include "mslibc.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <memory.h>

extern struct _struct_LL_Entry* _LL_Entry_New(void* data);
extern int _LL_Entry_GetSize(struct _struct_LL_Entry* head);
extern struct _struct_LL_Entry* _LL_Entry_Attach(struct _struct_LL_Entry* head,struct _struct_LL_Entry* attach);
extern void* _zalloc(size_t size);

typedef struct _struct_LL_Entry{
	struct _struct_LL_Entry*		prev;
	struct _struct_LL_Entry*		next;
	void*		data;
}_LL_Entry;

#pragma region Linked List
//
//typedef struct _struct_LL_Entry{
//	struct _struct_LL_Entry*		prev;
//	struct _struct_LL_Entry*		next;
//	void*		data;
//}_LL_Entry;
//
//_LL_Entry*		_LL_Entry_New			(void* data){
//	_LL_Entry* ret=(_LL_Entry*)malloc(sizeof(_LL_Entry));
//	memset(ret,0,sizeof(_LL_Entry));
//	ret->data=data;
//	return ret;
//}
//int			_LL_Entry_GetSize		(_LL_Entry* head){
//	_LL_Entry* cur=head;
//	int l=0;
//	if(cur==NULL)
//		return 0;
//	while(cur!=NULL){
//		l++;
//		cur=cur->next;
//	}
//	return l;
//}
//
///* attaches attach to LL that starts at head */
///* returns head for easy coding later lulz*/
//_LL_Entry*		_LL_Entry_Attach		(_LL_Entry* head,_LL_Entry* attach){
//	_LL_Entry* cur;
//	if(head==NULL)
//		head=attach; /* then attach is the first*/
//	/*find end of list*/
//	cur=head;
//	while(cur->next!=NULL){
//		cur=cur->next;
//	}
//	if(cur!=attach){
//		cur->next=attach;
//		attach->prev=cur;
//	}
//	return head;
//}
//
#pragma endregion

#pragma region This File Declarations
struct _struct_MS_DataImageRegistry{
	MS_DataImage**		images;
	int*				freed;
	int					length;
	int					size;
}*MS_REGISTRY;
typedef struct _struct_MS_DataImageRegistry MS_DataImageRegistry;
int				_MS_DataImageRegistry_Init			();
MS_DataImage*	_MS_DataImageRegistry_Register		(MS_DataImage* img);
MS_DataImage*	_MS_DataImageRegistry_Wrap			(WZLib_Image* wzImg);
int				_MS_DataImageRegistry_IncRefCount	(MS_DataImage* img);
int				_MS_DataImageRegistry_DecRefCount	(MS_DataImage* img);

int				_MS_Drawable_Init		(MS_Drawable* dobj,GLuint texID,int sizeW,int sizeH,int texSizeW,int texSizeH);
void			_MS_Drawable_Free		(MS_Drawable* dobj);

typedef struct _struct_MS_LifeEntry{
	char*		name;
	MS_Sprite*	spr;
	_LL_Entry*	container;
}MS_LifeEntry;
MS_LifeEntry*	_MS_LifeEntry_New		();
void			_MS_LifeEntry_Free		(MS_LifeEntry* ent);
int				_MS_Life_Init			(MS_Life* life);
int				_MS_Life_AddState		(MS_Life* life,char* state,MS_Sprite* spr);
MS_LifeEntry*	_MS_Life_GetState		(MS_Life* life,char* state);
int				_MS_Life_ChangeState	(MS_Life* life,char* state,MS_Sprite* newSpr);
int				_MS_Life_Update			(MS_Life* life);
int				_MS_Life_Draw			(MS_Life* life,int parentPos[2]);
void			_MS_Life_Free			(MS_Life* life);

int				_MS_Mob_Init			(MS_Mob* mob);

typedef struct _struct_MS_CharacterFrameElement{
}MS_CharacterFrameElement;
struct _struct_MS_Character{
	MS_Drawable		inh;
	MS_Life*		face;
	MS_Life*		head;
	MS_Life*		hair;
};
MS_Life**		_MS_Character_GetElementRef	(MS_Character* chr,MS_CharacterElementType el);
void			_MS_Character_SetElement	(MS_Character* chr,MS_CharacterElementType el,MS_Life* spr);
#pragma endregion

#pragma region Utilities
//void*	_zalloc(size_t size){
//	return memset(malloc(size),'\0',size);
//}
int _strcmp_optimized(char* str1,char* str2){
	int i=0;
	while(str1[i]!='\0' && str2[i]!='\0'){
		if(str1[i]!=str2[i])
			return 1!=1;
		i++;
	}
	if(str1[i]!=str2[i])
		return 1!=1;
	return 1==1;
}
#pragma endregion

int MS_Init(){
	_MS_DataImageRegistry_Init();
	return 0;
}

#pragma region MS_DataImageRegistry and MS_DataImage
int _MS_DataImageRegistry_Init(){
	MS_REGISTRY=(MS_DataImageRegistry*)_zalloc(sizeof(MS_DataImageRegistry));
	MS_REGISTRY->length=0;
	MS_REGISTRY->size=16;
	MS_REGISTRY->images=(MS_DataImage**)_zalloc(sizeof(MS_DataImage*)*MS_REGISTRY->size);
	MS_REGISTRY->freed=(int*)_zalloc(sizeof(int)*MS_REGISTRY->size);
	return 0;
}
MS_DataImage* _MS_DataImageRegistry_Wrap(WZLib_Image* img){
	if(img==NULL)
		return NULL;
	{
		int i=0;
		for(i=0;i<MS_REGISTRY->length;i++)
			if(MS_REGISTRY->images[i]->img==img)
				return MS_REGISTRY->images[i];
	}
	{
		MS_DataImage* msImg=(MS_DataImage*)_zalloc(sizeof(MS_DataImage));
		msImg->img=img;
		return _MS_DataImageRegistry_Register(msImg);
	}
	return NULL;
}
MS_DataImage* _MS_DataImageRegistry_Register(MS_DataImage* img){
	if(img==NULL)
		return NULL;
	if(img->registered)
		return img;
	{
		int i=0;
		for(i=0;i<MS_REGISTRY->length;i++){
			if(img->img==MS_REGISTRY->images[i]->img || img==MS_REGISTRY->images[i]){
				return MS_REGISTRY->images[i];
			}
		}
	}
	{
		if(MS_REGISTRY->length>=MS_REGISTRY->size){
			MS_REGISTRY->size*=2;
			MS_REGISTRY->images=(MS_DataImage**)realloc(MS_REGISTRY->images,sizeof(MS_DataImage*)*MS_REGISTRY->size);
			MS_REGISTRY->freed=(int*)realloc(MS_REGISTRY->freed,sizeof(int)*MS_REGISTRY->size);
		}
		MS_REGISTRY->images[MS_REGISTRY->length]=img;
		MS_REGISTRY->length++;
		img->registered=1;
		return img;
	}
	return img;
}
int _MS_DataImageRegistry_IncRefCount(MS_DataImage* img){
	img=_MS_DataImageRegistry_Register(img);
	if(img->refCount==0)
		if(img->img!=NULL && !img->img->inh.parsed)
			WZLib_Image_Parse(img->img);
	img->refCount++;
	return 0;
}
int _MS_DataImageRegistry_DecRefCount(MS_DataImage* img){
	img=_MS_DataImageRegistry_Register(img);
	if(img->refCount==0)
		return -1;
	img->refCount--;
	if(img->refCount==0);
	//TODO: free images upon refCount=0?
	return 0;
}
#pragma endregion

#pragma region MS_Object
#pragma endregion

#pragma region MS_WZLib_Object
/*MS_WZLib_Object* MS_WZLib_Object_New(WZLib_Object* obj){
	MS_WZLib_Object* ret=(MS_WZLib_Object*)_zalloc(sizeof(MS_WZLib_Object));
	ret->obj=obj;
	if(_strcmp_optimized(ret->obj->name,"3230303.img"))
		ret=ret;
	return ret;
}
int MS_WZLib_Object_Own(MS_WZLib_Object* wzObj,MS_Object* msObj){
	//TODO: add ownership locking here
	MS_Object* oldOwner=NULL;
	if(wzObj==NULL)
		return -1;
	if(_strcmp_optimized(wzObj->obj->name,"3230303.img"))
		oldOwner=oldOwner;
	oldOwner=wzObj->owner;
	if(!MS_Object_OnDeOwn(oldOwner,wzObj))
		return -3;
	if(!MS_Object_OnOwn(msObj,wzObj))
		return -4;
	wzObj->owner=msObj;
	return 0;
}
MS_Object* MS_WZLib_Object_Owner(MS_WZLib_Object* wzObj){
	if(wzObj==NULL)
		return NULL;
	return wzObj->owner;
}//*/
#pragma endregion

#pragma region MS_Drawable
int _MS_Drawable_Init(MS_Drawable* dobj,GLuint texID,int sizeW,int sizeH,int texSizeW,int texSizeH){
	dobj->textureID=texID;
	dobj->textureSize[0]=texSizeW;
	dobj->textureSize[1]=texSizeH;
	dobj->size[0]=sizeW;
	dobj->size[1]=sizeH;
	return 0;
}
int MS_Drawable_Draw(MS_Drawable* dobj,int parentPos[2]){
	float wr,hr; /*width, height ratio*/
	int drawX,drawY;
	wr=((float)dobj->size[0])/((float)dobj->textureSize[0]);
	hr=((float)dobj->size[1])/((float)dobj->textureSize[1]);
	if(parentPos!=NULL){
		drawX=parentPos[0]+dobj->position[0]-dobj->origin[0];
		drawY=parentPos[1]+dobj->position[1]-dobj->origin[1];
	}else{
		drawX=dobj->position[0]-dobj->origin[0];
		drawY=dobj->position[1]-dobj->origin[1];
	}
	glBindTexture(GL_TEXTURE_2D,dobj->textureID);

	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2f(drawX,drawY);
		glTexCoord2f(wr,0);
		glVertex2f(drawX+dobj->size[0],drawY);
		glTexCoord2f(wr,hr);
		glVertex2f(drawX+dobj->size[0],drawY+dobj->size[1]);
		glTexCoord2f(0,hr);
		glVertex2f(drawX,drawY+dobj->size[1]);
	glEnd();
}
void _MS_Drawable_Free(MS_Drawable* dobj){
	_MS_DataImageRegistry_DecRefCount(dobj->source);
}
#pragma endregion

#pragma region MS_Sprite
MS_Sprite* MS_Sprite_New(){
	MS_Sprite* spr=(MS_Sprite*)_zalloc(sizeof(MS_Sprite));
	return spr;
}
int MS_Sprite_LoadFromSubProperty(MS_Sprite* spr,WZLib_SubProperty* sub){
	int len=0;
	int i=0;
	int count=0;
	if(sub==NULL)
		return -1;
	len=WZLib_Object_GetSize(sub);
	spr->inh.source=_MS_DataImageRegistry_Wrap(sub->inh.parentImg);
	_MS_DataImageRegistry_IncRefCount(spr->inh.source);
	for(i=0;i<len;i++){
		WZLib_Property* obj=(WZLib_Property*)WZLib_Object_Get_i(sub,i);
		WZLib_PNGProperty* png=NULL;
		MS_Drawable* dr=NULL;
		_LL_Entry* newE=NULL;
		if(obj->propType!=WZLib_Prop_Canvas)
			continue;
		png=((WZLib_CanvasProperty*)obj)->png;
		if(!png->pngParsed)
			WZLib_PNGProperty_Parse(png);
		dr=(MS_Drawable*)_zalloc(sizeof(MS_Drawable));
		_MS_Drawable_Init(dr,png->texture,png->width,png->height,png->texWidth,png->texHeight);
		dr->source=(WZLib_Object*)png;
		newE=_LL_Entry_New((void*)dr);
		spr->frames=_LL_Entry_Attach(spr->frames,newE);
		{/*find origin and delay*/
			WZLib_CanvasProperty* canv=(WZLib_CanvasProperty*)obj;
			WZLib_PrimitiveProperty* prim=(WZLib_PrimitiveProperty*)WZLib_Object_Get_n((WZLib_Object*)canv,"origin");
			if(prim!=NULL && prim->inh.propType==WZLib_Prop_Vector){
				dr->origin[0]=prim->val.vecVal[0];
				dr->origin[1]=prim->val.vecVal[1];
			}
			prim=(WZLib_PrimitiveProperty*)WZLib_Object_Get_n((WZLib_Object*)canv,"delay");
			if(prim!=NULL && prim->inh.propType==WZLib_Prop_CompressedInt)
				dr->delay=prim->val.intVal;
			else
				dr->delay=100;
		}
		count++;
	}
	if(spr->frames!=NULL)
		spr->currentFrame=spr->frames;
	return count;
}
int MS_Sprite_Update(MS_Sprite* spr){
	clock_t now=clock();
	clock_t ticks=(now-spr->_lastUpdate)*1000/CLOCKS_PER_SEC;
	MS_Drawable* dr=NULL;
	if(spr->currentFrame==NULL)
		if(spr->frames==NULL)
			return -1;
		else
			spr->currentFrame=spr->frames;
	dr=(MS_Drawable*)spr->currentFrame->data;
	if(ticks>=dr->delay){
		if(spr->currentFrame->next==NULL)
			spr->currentFrame=spr->frames;
		else
			spr->currentFrame=spr->currentFrame->next;
		spr->_lastUpdate=now;
	}
	return 0;
}
int MS_Sprite_Draw(MS_Sprite* spr,int parentPos[2]){
	MS_Drawable* dr=NULL;
	int drawPos[2]={spr->inh.position[0],spr->inh.position[1]};
	if(spr->currentFrame==NULL)
		if(spr->frames==NULL)
			return -1;
		else
			spr->currentFrame=spr->frames;
	if(parentPos!=NULL){
		drawPos[0]+=parentPos[0];
		drawPos[1]+=parentPos[1];
	}
	dr=(MS_Drawable*)spr->currentFrame->data;
	MS_Drawable_Draw(dr,drawPos);
	return 0;
}
void MS_Sprite_Free(MS_Sprite* spr){
	_LL_Entry* cur=NULL;
	if(spr==NULL)
		return;
	_MS_Drawable_Free((MS_Drawable*)spr);
	cur=spr->frames;
	while(cur!=NULL){
		_LL_Entry* temp=cur;
		MS_Drawable* dr=(MS_Drawable*)cur->data;
		free(dr);
		cur=cur->next;
		free(temp);
	}
}
#pragma endregion

#pragma region MS_Life
MS_LifeEntry* _MS_LifeEntry_New(){
	return (MS_LifeEntry*)_zalloc(sizeof(MS_LifeEntry));
}
void _MS_LifeEntry_Free(MS_LifeEntry* ent){
	free(ent->name);
	MS_Sprite_Free(ent->spr);
}
int _MS_Life_Init(MS_Life* life){
	return 0;
}
int MS_Life_LoadFromSubProperty(MS_Life* life,WZLib_SubProperty* sub){
	_LL_Entry* cur=NULL;
	if(sub==NULL)
		return -1;
	life->inh.inh.source=_MS_DataImageRegistry_Wrap(sub->inh.parentImg);
	_MS_DataImageRegistry_IncRefCount(life->inh.inh.source);
	cur=sub->_firstChild;
	/*assume each subprop not named "info" or "icon" is a state*/
	while(cur!=NULL){
		WZLib_Property* thisP=(WZLib_Property*)cur->data;
		if(!_strcmp_optimized("info",thisP->inh.name) && !_strcmp_optimized("icon",thisP->inh.name)){
			MS_Sprite* spr=MS_Sprite_New();
			MS_Sprite_LoadFromSubProperty(spr,thisP);
			_MS_Life_AddState(life,thisP->inh.name,spr);
		}
		cur=cur->next;
	}
	return 0;
}
int _MS_Life_AddState(MS_Life* life,char* state,MS_Sprite* spr){
	MS_LifeEntry* newE=NULL;
	int slen=0;
	_LL_Entry* newL=NULL;
	if(state==NULL || spr==NULL)
		return -1;
	slen=strlen(state)+1;
	newE=_MS_LifeEntry_New();
	newE->name=(char*)_zalloc(sizeof(char)*slen);
	strcpy(newE->name,state);
	newE->spr=spr;
	newL=_LL_Entry_New((void*)newE);
	newE->container=newL;
	life->states=_LL_Entry_Attach(life->states,newL);
	if(life->currentState==NULL)
		life->currentState=life->states;
	return 0;
}
MS_LifeEntry* _MS_Life_GetState(MS_Life* life,char* state){
	MS_LifeEntry* ret=NULL;
	_LL_Entry* cur=life->states;
	while(cur!=NULL){
		MS_LifeEntry* temp=(MS_LifeEntry*)cur->data;
		if(_strcmp_optimized(state,temp->name)){
			ret=temp;
			break;
		}
		cur=cur->next;
	}
	return ret;
}
char* MS_Life_GetCurrentStateName(MS_Life* life){
	if(life->currentState==NULL)
		if(life->states==NULL)
			return NULL;
		else
			life->currentState=life->states;
	return ((MS_LifeEntry*)life->currentState->data)->name;
}
int _MS_Life_ChangeState(MS_Life* life,char* state,MS_Sprite* spr){
	MS_LifeEntry* ent=NULL;
	_LL_Entry* cont=NULL;
	ent=_MS_Life_GetState(life,state);
	if(ent==NULL)
		return -1;
	cont=ent->container;
	if(spr==NULL){/*kill this state*/
		_LL_Entry* prev=cont->prev;
		_LL_Entry* next=cont->next;
		prev->next=next;
		next->prev=prev;
		free(cont);
		_MS_LifeEntry_Free(ent);
		free(ent);
		return 0;
	}
	MS_Sprite_Free(ent->spr);
	ent->spr=spr;
	return 0;
}
int MS_Life_ChangeCurrentState(MS_Life* life,char* state){
	_LL_Entry* cur=NULL;
	if(life==NULL)
		return -1;
	cur=life->states;
	while(cur!=NULL){
		MS_LifeEntry* ent=(MS_LifeEntry*)cur->data;
		if(_strcmp_optimized(ent->name,state)){
			life->currentState=cur;
			return 0;
		}
		cur=cur->next;
	}
	return -1;
}
int _MS_Life_Update(MS_Life* life){
	MS_Sprite* spr=NULL;
	if(life==NULL)
		return -1;
	if(life->currentState==NULL)
		if(life->states==NULL)
			return -1;
		else
			life->currentState=life->states;
	spr=((MS_LifeEntry*)life->currentState->data)->spr;
	return MS_Sprite_Update(spr);
}
int _MS_Life_Draw(MS_Life* life,int parentPos[2]){
	MS_Sprite* spr=NULL;
	int drawPos[2]={life->inh.inh.position[0],life->inh.inh.position[1]};
	if(life->currentState==NULL)
		if(life->states==NULL)
			return -1;
		else
			life->currentState=life->states;
	if(parentPos!=NULL){
		drawPos[0]+=parentPos[0];
		drawPos[1]+=parentPos[1];
	}
	spr=((MS_LifeEntry*)life->currentState->data)->spr;
	MS_Sprite_Draw(spr,drawPos);
}
void _MS_Life_Free(MS_Life* life){
	_LL_Entry* cur=NULL;
	if(life==NULL)
		return;
	MS_Sprite_Free((MS_Sprite*)life);
	cur=life->states;
	while(cur!=NULL){
		MS_LifeEntry* ent=(MS_LifeEntry*)cur->data;
		_LL_Entry* temp=cur;
		_MS_LifeEntry_Free(ent);
		cur=cur->next;
		free(temp);
	}
	life->currentState=NULL;
	life->states=NULL;
}
#pragma endregion

#pragma region MS_Mob
int _MS_Mob_Init(MS_Mob* mob){
	_MS_Life_Init((MS_Life*)mob);
}
MS_Mob* MS_Mob_LoadFromImage(WZLib_Image* img){
	MS_Mob* ret=NULL;
	if(img==NULL)
		return NULL;
	ret=(MS_Mob*)_zalloc(sizeof(MS_Mob));
	if(!img->inh.parsed)
		WZLib_Image_Parse(img);
	if(MS_Life_LoadFromSubProperty((MS_Life*)ret,img->_sub)!=0)
		return NULL;
	return ret;
}
int MS_Mob_Update(MS_Mob* mob){
	return _MS_Life_Update((MS_Life*)mob);
}
int MS_Mob_Draw(MS_Mob* mob,int parentPos[2]){
	return _MS_Life_Draw((MS_Life*)mob,parentPos);
}
void MS_Mob_Free(MS_Mob* mob){
	_MS_Life_Free((MS_Life*)mob);
	WZLib_Image_Unparse((WZLib_Image*)mob->inh.inh.inh.source);
}
#pragma endregion

#pragma region MS_Character
/*chiz 'bout to get heavy*/
MS_Character* MS_Character_New(){
	return (MS_Character*)_zalloc(sizeof(MS_Character));
}
MS_Life** _MS_Character_GetElementRef(MS_Character* chr,MS_CharacterElementType el){
	MS_Life** ret=NULL;
	switch(el){
	case MS_CHARACTER_HEAD:
		ret=&(chr->head);
		break;
	case MS_CHARACTER_FACE:
		ret=&(chr->face);
		break;
	case MS_CHARACTER_HAIR:
		ret=&(chr->hair);
		break;
	}
	return ret;
}
MS_Life* MS_Character_GetElement(MS_Character* chr,MS_CharacterElementType el){
	return *(_MS_Character_GetElementRef(chr,el));
}
void _MS_Character_SetElement(MS_Character* chr,MS_CharacterElementType el,MS_Life* spr){
	MS_Life** newLife=_MS_Character_GetElementRef(chr,el);
	if(spr!=NULL)
		*newLife=spr;
}
int MS_Character_LoadElementFromImage(MS_Character* chr,MS_CharacterElementType el,WZLib_Image* img){
	int ret=0;
	MS_Life** current=NULL;
	MS_Life* currentLife=NULL;
	current=_MS_Character_GetElementRef(chr,el);
	if(current==NULL)
		return -1;
	if(img==NULL)
		return -2;
	currentLife=*current;
	if(currentLife!=NULL)
		_MS_Life_Free(currentLife); //send back to OEM state
	else{ //make a new one
		currentLife=(MS_Life*)_zalloc(sizeof(MS_Life));
		_MS_Life_Init(currentLife);
		*current=currentLife;
	}
	return MS_Life_LoadFromSubProperty(currentLife,img->_sub);
}
#pragma endregion