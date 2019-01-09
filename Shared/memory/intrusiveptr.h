#pragma once

// boost intrusive_ptr
// add 
// <Function><Name>intrusive_ptr&lt;.*&gt;::.*</Name><Action>NoStepInto</Action></Function>
// to $(VSFolder)\Common7\Packages\Debugger\Visualizers\default.natstepfilter
// to keep debugger from stepping into intrusive_ptr member functions.
template <class T> class intrusive_ptr {
	typedef intrusive_ptr this_type;

public:
	typedef T element_type;

	intrusive_ptr() : px(0) {
	}

	intrusive_ptr(T * p, bool add_ref = true) : px(p) {
		if (px != 0 && add_ref) intrusive_ptr_add_ref(px);
	}

	template <class U>
	intrusive_ptr(intrusive_ptr<U> const & rhs)
		: px(rhs.get()) {
		if (px != 0) intrusive_ptr_add_ref(px);
	}

	intrusive_ptr(intrusive_ptr const & rhs) : px(rhs.px) {
		if (px != 0) intrusive_ptr_add_ref(px);
	}

	~intrusive_ptr() {
		if (px != 0) intrusive_ptr_release(px);
	}

	template <class U> intrusive_ptr & operator=(intrusive_ptr<U> const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	// Move support
	intrusive_ptr(intrusive_ptr && rhs) : px(rhs.px) {
		rhs.px = 0;
	}

	intrusive_ptr & operator=(intrusive_ptr && rhs) {
		this_type(static_cast<intrusive_ptr &&>(rhs)).swap(*this);
		return *this;
	}

	template <class U> friend class intrusive_ptr;

	template <class U>
	intrusive_ptr(intrusive_ptr<U> && rhs)
		: px(rhs.px) {
		rhs.px = 0;
	}

	template <class U>
	intrusive_ptr & operator=(intrusive_ptr<U> && rhs) {
		this_type(static_cast<intrusive_ptr<U> &&>(rhs)).swap(*this);
		return *this;
	}

	intrusive_ptr & operator=(intrusive_ptr const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	operator bool() const {
		return px != 0;
	}

	intrusive_ptr & operator=(T * rhs) {
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

	void swap(intrusive_ptr & rhs) {
		T * tmp = px;
		px = rhs.px;
		rhs.px = tmp;
	}

private:
	T * px;
};

template <class T> inline bool operator==(intrusive_ptr<T> const & a, std::nullptr_t b) {
	return a.get() == 0;
}

template <class T> inline bool operator!=(intrusive_ptr<T> const & a, std::nullptr_t b) {
	return a.get() != 0;
}

template <class T, class U> inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b) {
	return a.get() == b.get();
}

template <class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b) {
	return a.get() != b.get();
}

template <class T, class U> inline bool operator==(intrusive_ptr<T> const & a, U * b) {
	return a.get() == b;
}

template <class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, U * b) {
	return a.get() != b;
}

template <class T, class U> inline bool operator==(T * a, intrusive_ptr<U> const & b) {
	return a == b.get();
}

template <class T, class U> inline bool operator!=(T * a, intrusive_ptr<U> const & b) {
	return a != b.get();
}

template <class T> inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b) {
	return std::less<T *>()(a.get(), b.get());
}

template <class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs) {
	lhs.swap(rhs);
}

// mem_fn support

template <class T> T * get_pointer(intrusive_ptr<T> const & p) {
	return p.get();
}

#include <atomic>
#include "../types.h"

struct thread_unsafe_counter {
	typedef int underlying;

	static int load(underlying const& counter) {
		return counter;
	}

	static void increment(underlying& counter) {
		++counter;
	}

	static int decrement(underlying& counter) {
		return --counter;
	}
};

struct thread_safe_counter {
	typedef std::atomic_int underlying;

	static int load(underlying const& counter) {
		return counter;
	}

	static void increment(underlying& counter) {
		++counter;
	}

	static int decrement(underlying& counter) {
		return --counter;
	}
};

template <class CounterPolicy = thread_safe_counter>
class SUEDE_API intrusive_ref_counter {
	typedef typename CounterPolicy::underlying underlying;

	template <class CounterPolicy>
	friend void intrusive_ptr_add_ref(intrusive_ref_counter<CounterPolicy>* counter);

	template <class CounterPolicy> 
	friend void intrusive_ptr_release(intrusive_ref_counter<CounterPolicy>* counter);

	underlying refs_ = 0;

public:
	virtual ~intrusive_ref_counter() {}
};

template <class CounterPolicy>
inline void intrusive_ptr_add_ref(intrusive_ref_counter<CounterPolicy>* counter) {
	CounterPolicy::increment(counter->refs_);
}

template <class CounterPolicy>
inline void intrusive_ptr_release(intrusive_ref_counter<CounterPolicy>* counter) {
	if (CounterPolicy::load(counter->refs_) > 0 && CounterPolicy::decrement(counter->refs_) == 0) {
		delete counter;
	}
}
