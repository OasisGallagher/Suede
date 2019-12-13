#pragma once
#include <iterator>

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long uint64;

typedef uint suede_guid;

#ifdef SUEDE_EXPORT
#define SUEDE_API __declspec(dllexport)
#else
#define SUEDE_API __declspec(dllimport)
#endif

#ifdef SUEDE_MATH_EXPORT
#define SUEDE_MATH_API __declspec(dllexport)
#else
#define SUEDE_MATH_API __declspec(dllimport)
#endif

template <class T>
class SuedeEnumerable {
public:
	typedef T iterator;
	typedef typename std::iterator_traits<T>::value_type value_type;

public:
	SuedeEnumerable(iterator first, iterator last)
		: first_(first), last_(last) {}

public:
	iterator begin() { return first_; }
	iterator end() { return last_; }

private:
	iterator first_, last_;
};
