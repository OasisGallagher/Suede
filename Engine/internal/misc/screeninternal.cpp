#include "screeninternal.h"
#include "internal/memory/memory.h"

Screen screenInstance(Memory::Create<ScreenInternal>());
