#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
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

