#pragma once
#include <vector>
#include <memory>

#include "types.h"
#include "memory/refptr.h"

// is std::shared_ptr.
template<class T> struct suede_is_shared_ptr : std::false_type {};
template<class T> struct suede_is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

// is ref_ptr
template<class T> struct suede_is_ref_ptr : std::false_type {};
template<class T> struct suede_is_ref_ptr<ref_ptr<T>> : std::true_type {};

// is raw ptr or shared_ptr or ref_ptr.
template <class T> struct suede_is_ptr {
	static const bool value = std::is_pointer<T>::value || suede_is_shared_ptr<T>::value || suede_is_ref_ptr<T>::value;
};

// is std::vector.
template <class T> struct suede_is_vector : public std::false_type {};
template <class T, class A> struct suede_is_vector<std::vector<T, A>> : public std::true_type {};
//

/**
 * @brief static ref_ptr cast.
 */
template <class T, class Ptr>
inline T suede_static_cast(const Ptr& p) {
	return static_cast<T::element_type*>(p.get());
}

/**
 * @brief dynamic ref_ptr cast.
 */
template<class T, class Ptr> T suede_dynamic_cast(const Ptr& p) {
	return dynamic_cast<T::element_type*>(p.get());
}

#define SUEDE_USE_NAMESPACE

#ifdef  SUEDE_USE_NAMESPACE
#define SUEDE_BEGIN_NAMESPACE	namespace Suede {
#define SUEDE_END_NAMESPACE		}
#else
#define SUEDE_BEGIN_NAMESPACE
#define SUEDE_END_NAMESPACE
#endif

#define SUEDE_MAX_DECALS		256
