// suede internal header
#pragma once
#include "../types.h"

class SUEDE_API PimplIdiom {
public:
	PimplIdiom(void* d, void(*destroyer)(void*)) : d_(d), destroyer_(destroyer) {}
	virtual ~PimplIdiom() { _destroy(); }

public: // internal ptr helpers
	template <class T>
	T* _rptr_impl() const { return (T*)d_; }

	template <class T>
	T* _rptr_impl(T*) const { return (T*)d_; }

	bool _d_equals_impl(void* d) { return d_ == d; }

protected:
	template <class T>
	typename T::Internal* _dptr_impl(T*) const {
		return (T::Internal*)(d_);
	}

	void _destroy() {
		destroyer_(d_);
		d_ = nullptr;
	}

protected:
	void* d_;
	void(*destroyer_)(void*);
};

#define SUEDE_DECLARE_IMPLEMENTATION(name)	public: \
	typedef class name ## Internal Internal; \
	private:

/** internal macro helpers */
// internal implementation ptr of this.
#define _suede_dptr()		_dptr_impl(this)

// implementation ptr of o.
#define _suede_rptr(o)			(o)->_rptr_impl(this)
#define _suede_ref_rptr(o)		(o).get()->_rptr_impl(this)

// internal implementation ptr of instance.
#define _suede_dinstance()	instance()->_dptr_impl(instance())

// implementation equals.
#define _suede_d_equals(o)	(((o) != nullptr) && (o)->_d_equals_impl(this))
