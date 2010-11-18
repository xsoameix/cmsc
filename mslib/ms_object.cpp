#include "ms_object.h"

#include <string.h>

namespace __MSLIB_NAMESPACE{
	Depth::Depth(){
		this->_size=0;
		memset((void*)this->_z,0,sizeof(int)*8);
	}
	int& Depth::operator [](unsigned int i){
		if(i>=8)
			return this->_z[0];
		return this->_z[i];
	}
	void Depth::operator =(int val){
		if(this->_size>7)
			return;
		this->_z[this->_size]=val;
		this->_size++;
	}
	bool Depth::operator()(Object* a, Object* b) {
		for(int i=0;i<8;i++) {
			if(a->depth[i]<b->depth[i])
				return true;
			if(a->depth[i]>b->depth[i])
				return false;
		}
		return a>b;
	}

	Object::Object(std::string name){
		this->_name=name;
	}
	void Object::renderThread(){
		SDL_CreateThread(&Object::renderThreadHelper,(void*)this);
	}
	int	Object::renderThreadHelper(void *obj){
		Object* objp=(Object*)obj;
		return objp->render();
	}
}
