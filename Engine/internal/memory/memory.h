#pragma once
#define ENABLE_CRT_MEMORY_CHECK

#if defined(ENABLE_CRT_MEMORY_CHECK) && (defined(_WIN32) || defined(_WIN64))
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef ENABLE_CRT_MEMORY_CHECK
#define MEMORY_CREATE(Ty)				new Ty()
#define MEMORY_CREATE_ARRAY(Ty, count)	new Ty[count]
#define MEMORY_RELEASE(pointer)			delete pointer
#define MEMORY_RELEASE_ARRAY(pointer)	delete[] pointer
#else
#define MEMORY_CREATE(Ty)				Memory::Create<Ty>()
#define MEMORY_CREATE_ARRAY(Ty, count)	Memory::CreateArray<Ty>(count)
#define MEMORY_RELEASE(pointer)			Memory::Release(pointer)
#define MEMORY_RELEASE_ARRAY(pointer)	Memory::ReleaseArray(pointer)
#endif

class Memory {
public:
	template <class Ty>
	static Ty* Create() {
		return new Ty();
	}

	template <class Ty>
	static void Release(Ty* pointer) {
		delete pointer;
	}

	template <class Ty>
	static Ty* CreateArray(size_t n) {
		return new Ty[n];
	}

	template <class Ty>
	static void ReleaseArray(Ty* pointer) {
		delete[] pointer;
	}
};

