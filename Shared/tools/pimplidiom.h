// suede internal header
#pragma once
#include "../types.h"

class SUEDE_API PimplIdiom {
public:
	PimplIdiom(void* d, void(*destroyer)(void*)) : d_(d), destroyer_(destroyer) {}
	virtual ~PimplIdiom() { _destroy(); }

public: // internal ptr helpers
	void* d_;

protected:
	void _destroy() {
		destroyer_(d_);
		d_ = nullptr;
	}

protected:
	void(*destroyer_)(void*);
};

#define SUEDE_DECLARE_IMPLEMENTATION(name)	public: \
	typedef class name ## Internal Internal; \
	private:

/** internal macro helpers */
#define __suede_dtype(ptr)	std::remove_pointer<decltype(ptr)>::type

// internal implementation ptr of this.
#define _suede_dptr()		((__suede_dtype(this)::Internal*)d_)

// internal implementation ptr of instance.
#define _suede_dinstance()	((__suede_dtype(instance())::Internal*)instance()->d_)
