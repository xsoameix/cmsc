#ifndef __MS_UTILS_H
#define __MS_UTILS_H
#include "ms_config.h"

#include <string>

#define NULLSTRHANDLE(x) (x==NULL?"":x)
#define NSH(x) NULLSTRHANDLE(x)

#include "wzlibc.h"
WZLib_ValUnion _get(WZLib_Object* parent,char* name);

#include <sstream>
namespace StringConversion{
	template<typename _Target,typename _Source> _Target convert(_Source str){
		std::stringstream ss;
		_Target val;
		ss<<str;
		ss>>val;
		return val;
	}
};

#endif