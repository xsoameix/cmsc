#include "ms_utils.h"

WZLib_ValUnion _get(WZLib_Object* parent,char* name){
	WZLib_PrimitiveProperty* prop=(WZLib_PrimitiveProperty*)WZLib_Object_Get_n(parent,name);
	if(prop==NULL){
		WZLib_ValUnion v;
		v.strVal=NULL;
		return v;
	}
	return prop->val;
}