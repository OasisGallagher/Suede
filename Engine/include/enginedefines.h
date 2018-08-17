#pragma once
#include <memory>
#include "types.h"

#define SUEDE_DEFINE_OBJECT_POINTER(Ty)	typedef std::shared_ptr<class I ## Ty> Ty
#define SUEDE_DECLARE_OBJECT_CREATER(Ty)	SUEDE_API Ty New ## Ty()

#define SUEDE_DEFINE_CUSTOM_OBJECT_POINTER(Ty)			class SUEDE_API Ty : public std::shared_ptr<I ## Ty>
#define SUEDE_IMPLEMENT_CUSTOM_OBJECT_POINTER(Ty)		public: \
																Ty() {} \
																\
																Ty(nullptr_t) {} \
																\
																template<class U> \
																Ty(U* px) : std::shared_ptr<I ## Ty>(px) {} \
																\
																template<class T, class U> \
																Ty(const std::shared_ptr<U>& right, T *px) : std::shared_ptr<I ## Ty>(right, px) {} \
																\
																template <class T, class Deleter, class Allocator> \
																Ty(T* ptr, Deleter deleter, Allocator allocator) : std::shared_ptr<I ## Ty>(ptr, deleter, allocator) {} \
																\
																template<class _Ty2> \
																Ty(const shared_ptr<_Ty2>& _Other) : std::shared_ptr<I ## Ty>(_Other) { } \
																\
																template<class _Ty2> \
																Ty& operator=(const std::shared_ptr<_Ty2>& _Right) { \
																	std::shared_ptr<I ## Ty>(_Right).swap(*this); \
																	return (*this);\
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

/**
 * @brief static shared_ptr cast.
 */
template<class T, class Ptr>
inline T suede_static_cast(const Ptr& ptr) {
	return T(ptr, static_cast<typename T::element_type*>(ptr.get()));
}

/**
 * @brief dynamic shared_ptr cast.
 */
template<class T, class Ptr>
inline T suede_dynamic_cast(const Ptr& ptr) {
	typedef typename T::element_type Element;
	Element* p = dynamic_cast<Element*>(ptr.get());
	if (p != nullptr) {
		return T(ptr, p);
	}

	return T();
}
