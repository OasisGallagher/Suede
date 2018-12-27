#pragma once
#include <memory>
#include <vector>

#include "types.h"

#define SUEDE_DEFINE_OBJECT_POINTER(Ty)		typedef std::shared_ptr<class I ## Ty> Ty;
#define SUEDE_DECLARE_OBJECT_CREATER(Ty)	SUEDE_API Ty New ## Ty();

template <class T>
using suede_weak_ref = std::weak_ptr<typename T::element_type>;

// is std::shared_ptr.
template<class T> struct suede_is_shared_ptr : std::false_type {};
template<class T> struct suede_is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

// is raw ptr or shared_ptr.
template <class T> struct suede_is_ptr {
	static const bool value = std::is_pointer<T>::value || suede_is_shared_ptr<T>::value;
};

// is std::vector.
template <class T> struct suede_is_vector : public std::false_type {};
template <class T, class A> struct suede_is_vector<std::vector<T, A>> : public std::true_type {};
//

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
