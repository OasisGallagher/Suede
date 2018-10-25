#pragma once
#include <memory>
#include "types.h"

#define SUEDE_DEFINE_OBJECT_POINTER(Ty)		typedef std::shared_ptr<class I ## Ty> Ty;
#define SUEDE_DECLARE_OBJECT_CREATER(Ty)	SUEDE_API Ty New ## Ty();

#define SUEDE_USE_NAMESPACE

#ifdef  SUEDE_USE_NAMESPACE
#define SUEDE_BEGIN_NAMESPACE	namespace Suede {
#define SUEDE_END_NAMESPACE		}
#else
#define SUEDE_BEGIN_NAMESPACE
#define SUEDE_END_NAMESPACE
#endif

#define SUEDE_MAX_DECALS		256

/**
 * @brief static shared_ptr cast.
 */
template <class T, class Ptr>
inline T suede_static_cast(const Ptr& ptr) {
	return T(ptr, static_cast<typename T::element_type*>(ptr.get()));
}

/**
 * @brief dynamic shared_ptr cast.
 */
template <class T, class Ptr>
inline T suede_dynamic_cast(const Ptr& ptr) {
	typedef typename T::element_type Element;
	Element* p = dynamic_cast<Element*>(ptr.get());
	if (p != nullptr) {
		return T(ptr, p);
	}

	return T();
}
