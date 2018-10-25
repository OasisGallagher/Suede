#pragma once

template <class T>
class TypeID {
	// dummy_ must not have a const type.  Otherwise an overly eager
	// compiler (e.g. MSVC 7.1 & 8.0) may try to merge
	// TypeIdHelper<T>::dummy_ for different Ts as an "optimization".
	static bool dummy_;

public:
	static intptr_t value() { return (intptr_t)&dummy_; }
};

template <class T>
bool TypeID<T>::dummy_ = false;
