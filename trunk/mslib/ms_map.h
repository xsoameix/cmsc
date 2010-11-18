#ifndef __MS_MAP_H
#define __MS_MAP_H
#include "ms_config.h"

#include <vector>
#include <set>

#include "SDLAnim/SDLAnim.h"
#include "wzlibc.h"

#include "ms_object.h"

namespace __MSLIB_NAMESPACE{
	class Map:public Object{
	public:

		class Portal:public Object{
		public:
			bool				render	();
			void				draw	(SDL_Surface* surf,SDLAnim::PxPoint* p);
			void				free	();
			__MSLIB_CTOR		Portal	(WZLib_SubProperty* node);
			//--------
			int					pt;
			std::string			pn;
			int					tm;
			std::string			tn;
			int					hi, vi;
			std::string			script;
			int					onlyOnce;
			int					uDelay;
			int					hideTooltip;
			int					frame;
			int					ani;
			int					delay;
		};

		class Foothold: public Object{
		public:
			bool				render		();
			void				draw		(SDL_Surface* surf,SDLAnim::PxPoint* p);
			void				free		();
			__MSLIB_CTOR		Foothold	(WZLib_SubProperty* node);
			//--------
			Foothold			*nextFH,*prevFH;
			int					x1, y1, x2, y2;
			double				dir, len;
			int					id, nextID, prevID;
			int					layer, group;
			double				force;
			bool				forbid;
			bool				walk;
		};

		class Ladder: public Object{
		public:
			bool				render		();
			void				draw		(SDL_Surface* surf,SDLAnim::PxPoint* p);
			void				free		();
			__MSLIB_CTOR		Ladder		(WZLib_SubProperty* node);
			//--------
			int					x, y1, y2;
			int					l, page, uf;
		};

		class Tile: public Object{
		public:
			bool			render	();
			void			draw	(SDL_Surface* surf,SDLAnim::PxPoint* p);
			void			free	();
			__MSLIB_CTOR	Tile	(WZLib_SubProperty* node);
			//--------

		};

		class Object:public __MSLIB_NAMESPACE::Object{
		public:
			bool				render	();
			void				draw	(SDL_Surface* surf,SDLAnim::PxPoint* p);
			void				free	();
			__MSLIB_CTOR		Object	(WZLib_SubProperty* node);
			//--------
			bool				f;
		};
	protected:
		std::vector<Portal*>	_portals;
		std::vector<Foothold*>	_footholds;
		std::vector<Ladder*>	_ladders;
		std::set<__MSLIB_NAMESPACE::Object*,__MSLIB_NAMESPACE::Depth>	_objects;
		WZLib_SubProperty*		_dataSub;
		WZLib_Image*			_mapImg;
	public:
		__MSLIB_CTOR			Map		(WZLib_Image* mapImg);
		bool					render	();
		void					draw	(SDL_Surface* surf,SDLAnim::PxPoint* p);
		void					free	();
		void					update	();
	};
}
#endif
