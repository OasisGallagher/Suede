#pragma once
#include <string>

template <class T>
class TypeID {
#ifndef _DEBUG
	// dummy_ must not have a const type. 
	// Otherwise an overly eager compiler (e.g. MSVC 7.1 & 8.0) may try to merge
	// TypeID<T>::dummy_ for different Ts as an "optimization".
	static bool dummy_;
#endif

public:
#ifndef _DEBUG
	static intptr_t value() { return (intptr_t)&dummy_; }
#endif

	static const char* string() {
#ifdef _DEBUG
		return typeid(T).name();
#else
		static std::string n = std::to_string(value());
		return n.c_str();
#endif
	}
};

#ifndef _DEBUG
template <class T>
bool TypeID<T>::dummy_ = false;
#endif
