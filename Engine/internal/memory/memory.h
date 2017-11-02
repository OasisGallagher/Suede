#pragma once

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

