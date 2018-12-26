#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Guard.h>

#include "debug/debug.h"

#define ZTHREAD_LOCK_SCOPE(mutex)	ZThread::Guard<ZThread::Mutex> _suedeGuard(mutex)
#define ZTHREAD_ASSERT_MAIN()		(ZThread::Thread::isMainThread() || (Debug::LogError("\"%s\" must be called in main thread.", __func__), 0))
