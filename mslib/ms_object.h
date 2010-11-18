#ifndef __MS_OBJECT_H
#define __MS_OBJECT_H
#include "ms_config.h"

#include <string>
#include "SDL/SDL.h"
#include "SDLAnim/SDLAnim.h"

namespace __MSLIB_NAMESPACE{

	class Object;
	class Depth{
	protected:
		int		_size;
		int		_z[8];
	public:
		__MSLIB_CTOR	Depth();
		enum _enum_depth{
			Back,
			Layer,
			Front,
			UI,
		};
		enum _enum_depthObjects{
			Objects,
			Tiles,
			Players,
		};
		//operators
		int& operator[](unsigned int i);
		void operator=(int val);
		//operator for std::set
		bool			operator()(Object* a,Object* b);
	};

	class Object:public SDLAnim::Sprite{
	protected:
		std::string	_name;
		static int		renderThreadHelper	(void* obj);
	public:
		bool			debug;
		Depth			depth;
		//---------------
		__MSLIB_CTOR	Object	(std::string name);
		virtual	bool	render	()=0;
		void			renderThread	();
		void			draw	(SDL_Surface* surf,SDLAnim::PxPoint* p)=0;
		virtual	void	free	()=0;
	};
}

#endif
