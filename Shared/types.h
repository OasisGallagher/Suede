#pragma once

typedef size_t suede_typeid;

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long uint64;

#ifdef SUEDE_EXPORT
#define SUEDE_API __declspec(dllexport)
#else
#define SUEDE_API __declspec(dllimport)
#endif

template <class T>
class SuedeEnumerable {
public:
	typedef T iterator;

public:
	SuedeEnumerable(iterator first, iterator last)
		: first_(first), last_(last) {}

public:
	iterator begin() { return first_; }
	iterator end() { return last_; }

private:
	iterator first_, last_;
};
