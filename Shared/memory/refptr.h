#pragma once

template<class T> class ref_ptr {
	typedef ref_ptr this_type;

public:
	typedef T element_type;

	ref_ptr() : px(0) {
	}

	ref_ptr(T * p, bool add_ref = true) : px(p) {
		if (px != 0 && add_ref) ref_ptr_add(px);
	}

	template<class U>
	ref_ptr(ref_ptr<U> const & rhs)
		: px(rhs.get()) {
		if (px != 0) ref_ptr_add(px);
	}

	ref_ptr(ref_ptr const & rhs) : px(rhs.px) {
		if (px != 0) ref_ptr_add(px);
	}

	~ref_ptr() {
		if (px != 0) ref_ptr_release(px);
	}

	template<class U> ref_ptr & operator=(ref_ptr<U> const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	// Move support
	ref_ptr(ref_ptr && rhs) : px(rhs.px) {
		rhs.px = 0;
	}

	ref_ptr & operator=(ref_ptr && rhs) {
		this_type(static_cast<ref_ptr &&>(rhs)).swap(*this);
		return *this;
	}

	template<class U> friend class ref_ptr;

	template<class U>
	ref_ptr(ref_ptr<U> && rhs)
		: px(rhs.px) {
		rhs.px = 0;
	}

	template<class U>
	ref_ptr & operator=(ref_ptr<U> && rhs) {
		this_type(static_cast<ref_ptr<U> &&>(rhs)).swap(*this);
		return *this;
	}

	ref_ptr & operator=(ref_ptr const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	operator bool() const {
		return px != 0;
	}

	ref_ptr & operator=(T * rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	void reset() {
		this_type().swap(*this);
	}

	void reset(T * rhs) {
		this_type(rhs).swap(*this);
	}

	void reset(T * rhs, bool add_ref) {
		this_type(rhs, add_ref).swap(*this);
	}

	T * get() const {
		return px;
	}

	T * detach() {
		T * ret = px;
		px = 0;
		return ret;
	}

	T & operator*() const {
		return *px;
	}

	T * operator->() const {
		return px;
	}

	void swap(ref_ptr & rhs) {
		T * tmp = px;
		px = rhs.px;
		rhs.px = tmp;
	}

private:
	T * px;
};

template<class T> inline bool operator==(ref_ptr<T> const & a, std::nullptr_t b) {
	return a.get() == 0;
}

template<class T> inline bool operator!=(ref_ptr<T> const & a, std::nullptr_t b) {
	return a.get() != 0;
}

template<class T, class U> inline bool operator==(ref_ptr<T> const & a, ref_ptr<U> const & b) {
	return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(ref_ptr<T> const & a, ref_ptr<U> const & b) {
	return a.get() != b.get();
}

template<class T, class U> inline bool operator==(ref_ptr<T> const & a, U * b) {
	return a.get() == b;
}

template<class T, class U> inline bool operator!=(ref_ptr<T> const & a, U * b) {
	return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, ref_ptr<U> const & b) {
	return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, ref_ptr<U> const & b) {
	return a != b.get();
}

template<class T> inline bool operator<(ref_ptr<T> const & a, ref_ptr<T> const & b) {
	return std::less<T *>()(a.get(), b.get());
}

template<class T> void swap(ref_ptr<T> & lhs, ref_ptr<T> & rhs) {
	lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(ref_ptr<T> const & p) {
	return p.get();
}

#include <atomic>
#include "../types.h"

class SUEDE_API intrusive_ref_counter {
	std::atomic<int> refs_ = 0;

	friend void ref_ptr_add(intrusive_ref_counter* counter);
	friend void ref_ptr_release(intrusive_ref_counter* counter);

public:
	virtual ~intrusive_ref_counter() {}
};

inline void ref_ptr_add(intrusive_ref_counter* counter) {
	++counter->refs_;
}

inline void ref_ptr_release(intrusive_ref_counter* counter) {
	if (counter->refs_ > 0 && --counter->refs_ == 0) {
		delete counter;
	}
}

template <class T, class... Args>
inline ref_ptr<T> make_ref(Args... args) {
	return MEMORY_NEW(T, args...);
}
