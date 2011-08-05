#ifndef __MSLIBC_H
#define __MSLIBC_H

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include <time.h>

#define WZLIB_HAVE_OPENGL
#include "wzlibc.h"

#	define	MS_DRAWABLE(obj)	((MS_Drawable*)obj)

#	define	MS_ERR_CANNOT_OWN	-65535

	int					MS_Init					();

	struct _struct_LL_Entry;
	struct _struct_MS_WZLib_Object;

	typedef struct _struct_MS_Object{
		char*			name;
	}MS_Object;

	typedef struct _struct_MS_DataImage{
		WZLib_Image*	img;
		unsigned int	refCount;
		int				registered;
	}MS_DataImage;

	typedef struct _struct_MS_Drawable{
		MS_Object		inh;
		int				position[2];
		int				origin[2];
		GLuint			textureID;
		int				textureSize[2];
		int				size[2];
		int				delay;
		MS_DataImage*	source;
	}MS_Drawable;

	int			MS_Drawable_Draw				(MS_Drawable* dobj,int parentPos[2]);

	typedef struct _struct_MS_Sprite{
		MS_Drawable					inh;
		struct _struct_LL_Entry*	frames;  /*data is MS_Drawable*/
		struct _struct_LL_Entry*	currentFrame;
		clock_t						_lastUpdate;
	}MS_Sprite;

	MS_Sprite*	MS_Sprite_New					();
	int			MS_Sprite_LoadFromSubProperty	(MS_Sprite* spr,WZLib_SubProperty* sub);
	int			MS_Sprite_Update				(MS_Sprite* spr);
	int			MS_Sprite_Draw					(MS_Sprite* spr,int parentPos[2]);
	void		MS_Sprite_Free					(MS_Sprite* spr);

	/*holds a number of named states, each corresponding to an MS_Sprite*/
	typedef struct _struct_MS_Life{
		MS_Sprite					inh;
		struct _struct_LL_Entry*	states;
		struct _struct_LL_Entry*	currentState;
	}MS_Life;

	int			MS_Life_ChangeCurrentState		(MS_Life* life,char* state);
	int			MS_Life_LoadFromSubProperty		(MS_Life* life,WZLib_SubProperty* sub);
	char*		MS_Life_GetCurrentStateName		(MS_Life* life);

	typedef struct _struct_MS_Mob{
		MS_Life				inh;
	}MS_Mob;

	MS_Mob*		MS_Mob_LoadFromImage	(WZLib_Image* img);
	int			MS_Mob_Update			(MS_Mob* mob);
	int			MS_Mob_Draw				(MS_Mob* mob,int parentPos[2]);
	void		MS_Mob_Free				(MS_Mob* mob);

	/*chiz here gets complicated, so leave it for further definition*/
	struct _struct_MS_Character;
	typedef struct _struct_MS_Character MS_Character;

	typedef enum _enum_MS_CharacterElementType{
		MS_CHARACTER_HEAD,
		MS_CHARACTER_FACE,
		MS_CHARACTER_HAIR,
		MS_CHARACTER_BODY,
	}MS_CharacterElementType;

	MS_Character*	MS_Character_New					();
	MS_Life*		MS_Character_GetElement				(MS_Character* chr,MS_CharacterElementType el);
	int				MS_Character_LoadElementFromImage	(MS_Character* chr,MS_CharacterElementType el,WZLib_Image* img);
	void			MS_Character_Free					(MS_Character* chr);


#ifdef __cplusplus
}
#endif

#endif