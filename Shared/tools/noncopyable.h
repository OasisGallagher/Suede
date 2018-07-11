#pragma once
#include "../types.h"

class SUEDE_API NonCopyable {
	NonCopyable(const NonCopyable&);
	const NonCopyable& operator=(const NonCopyable&);

protected:
	NonCopyable() {}
	~NonCopyable() {}
};
