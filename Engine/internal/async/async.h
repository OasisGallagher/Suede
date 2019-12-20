#pragma once
#include <cassert>

#include <ZThread/Mutex.h>
#include <ZThread/Guard.h>
#include <ZThread/Thread.h>

#define ZTHREAD_LOCK_SCOPE(mutex)	ZThread::Guard<ZThread::Mutex> _suedeGuard(mutex)
