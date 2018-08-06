#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Guard.h>

#define ZTHREAD_LOCK_SCOPE(mutex)	ZThread::Guard<ZThread::Mutex> _Guard(mutex)
