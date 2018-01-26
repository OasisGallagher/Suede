#pragma once

#define MEMORY_CREATE(T)				Memory::Create<T>()
#define MEMORY_CLONE(T, other)			Memory::Clone<T>(other)
#define MEMORY_CREATE_ARRAY(T, count)	Memory::CreateArray<T>(count)
#define MEMORY_RELEASE(pointer)			Memory::Release(pointer)
#define MEMORY_RELEASE_ARRAY(pointer)	Memory::ReleaseArray(pointer)

#include <memory>

class Memory {
public:
	template <class T>
	static T* Create() {
		return new T;
	}

	template <class T>
	static T* Clone(T* other) {
		return new T(*other);
	}

	template <class T>
	static void Release(T* pointer) {
		delete pointer;
	}

	template <class T>
	static T* CreateArray(size_t n) {
		return new T[n];
	}

	template <class T>
	static void ReleaseArray(T* pointer) {
		delete[] pointer;
	}

	static void* malloc(size_t size) {
#pragma push_macro("new")
#undef new
		return ::operator new(size);
#pragma pop_macro("new")
	}

	static void free(void* p) {
		::operator delete(p);
	}
};

template <class T> class Allocator;

template <>
class Allocator<void> {
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	typedef void value_type;

	template <class U>
	struct rebind {
		typedef Allocator<U> other;
	};
};

template <class T>
class Allocator {
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

public:
	template <class U>
	struct rebind { typedef Allocator<U> other; };

public:
	Allocator() {}
	template <class U>
	Allocator(const Allocator<U>&) {}

public:
	pointer address(reference x) { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	pointer allocate(size_type size, Allocator<void>::const_pointer hint = 0) {
		return (pointer)Memory::malloc(size * sizeof(T));
	}

	void deallocate(pointer p, size_type n) {
		Memory::free(p);
	}

	void deallocate(void *p, size_type n) {
		Memory::free(p);
	}

	size_type max_size() const {
		return size_t(-1) / sizeof(value_type);
	}

	void construct(pointer p, const T& val) {
		new((void*)(p)) T(val);
	}

	void construct(pointer p) {
		new((void*)(p)) T();
	}

	void destroy(pointer p) { destruct(p); }

private:
	template <class T> void destruct(T* ptr) { ptr->~T(); }
	//template <> void destruct(char*) {}
	//template <> void destruct(wchar_t*) {}
};

struct Deleter {
	template <class T>
	void operator()(T *p) {
		Memory::Release(p);
	}
};
