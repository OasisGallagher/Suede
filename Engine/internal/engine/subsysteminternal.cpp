#include "subsysteminternal.h"

Subsystem::Subsystem(void* d) : PimplIdiom(d, t_delete<SubsystemInternal>) {
}
