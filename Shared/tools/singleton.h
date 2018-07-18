#pragma once

// TODO: Thead safe.
// uncomment these two line will bring ZThread out....
//#include "../3rdparty/zthread-2.3.2/include/ZThread/Guard.h"
//#include "../3rdparty/zthread-2.3.2/include/ZThread/FastMutex.h"

#include <cassert>
#include "noncopyable.h"
//
// This policy controls how an object is instantiated
// as well as how and when its destroyed. Phoenix-style
// singletons are not supported easily with type of policy, 
// this is intentional since I do not believe that is in
// the true spirit of a singleton.
//
// InstantiationPolicContract {
//
//  create(pointer_type&)
//
// }

/**
* @class LocalStaticInstantiation
* @author Eric Crahen <http://www.code-foo.com>
* @date <2003-07-16T17:57:45-0400>
* @version 2.2.0
*
* The LocalStaticInstantiation policy allows the creation
* and lifetime of an instance of a particular type
* to be managed using static local values. This will
* abide by the standard C++ rules for static objects
* lifetimes.
*/
class LocalStaticInstantiation {
protected:

	/**
	* Create an instance of an object, using a local static. The
	* object will be destroyed by the system.
	*
	* @param ptr reference to location to receive the address
	* of the allocated object
	*/
	template <class T>
	static void create(T*& ptr) {

		static T instance;
		ptr = &instance;

	}

};

//! Helper class
template <class T>
class StaticInstantiationHelper {
	//! Friend class
	friend class StaticInstantiation;
	//! Holder
	static T instance;

};

template <class T>
T StaticInstantiationHelper<T>::instance;

/**
* @class StaticInstantiation
* @author Eric Crahen <http://www.code-foo.com>
* @date <2003-07-16T17:57:45-0400>
* @version 2.2.0
*
* The StaticInstantiation policy allows the creation
* and lifetime of an instance of a particular type
* to be managed using static instantiation. This will
* abide by the standard C++ rules for static objects
* lifetimes.
*/
class StaticInstantiation {
protected:

	/**
	* Create an instance of an object using by simply allocating it statically.
	*
	* @param ptr reference to location to receive the address
	* of the allocated object
	*/
	template <class T>
	static void create(T*& ptr) {
		ptr = &StaticInstantiationHelper<T>::instance;
	}

};

//! SingletonDestroyer
template <class T>
class Destroyer {

	T* doomed;

public:

	Destroyer(T* q) : doomed(q) {
		assert(doomed);
	}

	~Destroyer();

};

template <class T>
Destroyer<T>::~Destroyer() {

	try {

		if (doomed)
			delete doomed;

	}
	catch (...) {}

	doomed = 0;

}


/**
* @class LazyInstantiation
* @author Eric Crahen <http://www.code-foo.com>
* @date <2003-07-16T17:57:45-0400>
* @version 2.2.0
*
* The LazyInstantiation policy allows the creation
* and lifetime of an instance of a particular type
* to be managed using dynamic allocation and a singleton
* destroyer. This will abide by the standard C++ rules
* for static objects lifetimes.
*/
class LazyInstantiation {
protected:

	/**
	* Create an instance of an object, using new, that will be
	* destroyed when an associated Destroyer object (allocated
	* statically) goes out of scope.
	*
	* @param ptr reference to location to receive the address
	* of the allocated object
	*/
	template <class T>
	static void create(T*& ptr) {

		ptr = new T;
		static Destroyer<T> destroyer(ptr);

	}

};


/**
* @class Singleton
* @author Eric Crahen <http://www.code-foo.com>
* @date <2003-07-16T17:57:45-0400>
* @version 2.2.0
*
* Based on the work of John Vlissidles in his book 'Pattern Hatching'
* an article by Douglas Schmidtt on double-checked locking and policy
* templates described by Andrei Alexandrescu.
*
* This is a thread safe wrapper for creating Singleton classes. The
* synchronization method and instantiation methods can be changed
* easily by specifying different policy implementations as the
* templates parameters.
*
* @code
*
* // Most common Singleton
* Singletion<LonesomeType>
*
* // Singleton that uses static storage
* Singletion<LonesomeType, StaticInstantiation>
*
* // Single-threaded singleton that uses static storage (Meyers-like)
* Singletion<LonesomeType, LocalStaticInstantiation, NotLocked>
*
* @endcode
*/
template <class T, class InstantiationPolicy = LazyInstantiation> //, class LockType = ZThread::FastMutex>
class Singleton : private InstantiationPolicy, private NonCopyable {
public:
	/**
	* Provide access to the single instance through double-checked locking
	*
	* @return T* single instance
	*/
	static T* get();

};

template <class T, class InstantiationPolicy>//, class LockType>
T* Singleton<T, InstantiationPolicy>/*, LockType>*/::get() {

	// Uses local static storage to avoid static construction
	// sequence issues. (regaring when the lock is created)
	static T* ptr = 0;
	//static LockType lock;

	if (!ptr) {

		//ZThread::Guard<LockType, ZThread::LockedScope> g(lock);
		//if (!ptr)
		InstantiationPolicy::create(ptr);

	}

	return const_cast<T*>(ptr);

}