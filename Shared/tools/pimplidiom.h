// suede internal header
#pragma once
#include "../types.h"

template <class T>
static void t_delete(void* ptr) {
	delete ((T*)ptr);
}

class SUEDE_API PimplIdiom {
public:
	PimplIdiom(void* d, void(*destroyer)(void*)) : d_(d), destroyer_(destroyer) {}
	virtual ~PimplIdiom() { destroyer_(d_); }

public:
	void* const d_;

private:
	void(*destroyer_)(void*);
};

#define SUEDE_DECLARE_IMPLEMENTATION(name)	public: \
	typedef class name ## Internal Internal; \
	private:

/** Internal macro helpers */

#define __suede_dptr_impl(x)	((Internal*)x->d_)

// Get internal implementation ptr of wrapper.
#define _suede_dptr()			__suede_dptr_impl(this)
#define _suede_doptr(ptr)		__suede_dptr_impl(ptr)

// Get internal implementation ptr of instance.
#define _suede_dinstance()		__suede_dptr_impl(instance())

// Cast base internal implementation ptr to sub type.
#define _suede_rptr(o)			((std::remove_pointer<decltype(o)>::type::Internal*)(o)->d_)

//
